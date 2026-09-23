#include "DecoratorTiledImage.h"
#include "../../Include/RmlUi/Core/Element.h"
#include "../../Include/RmlUi/Core/Geometry.h"
#include "../../Include/RmlUi/Core/MeshUtilities.h"
#include "../../Include/RmlUi/Core/RenderManager.h"
#include <cmath>
#include <limits>

namespace Rml {

struct TiledImageElementData {
	Geometry geometry;
	BoxArea paint_area = BoxArea::Auto;
	float image_position_x = std::numeric_limits<float>::quiet_NaN();
	float image_position_y = std::numeric_limits<float>::quiet_NaN();
};

DecoratorTiledImage::DecoratorTiledImage() {}

DecoratorTiledImage::~DecoratorTiledImage() {}

bool DecoratorTiledImage::Initialise(const Tile& _tile, Texture _texture)
{
	tile = _tile;
	tile.texture_index = AddTexture(_texture);
	return (tile.texture_index >= 0);
}

DecoratorDataHandle DecoratorTiledImage::GenerateElementData(Element* element, BoxArea paint_area) const
{
	// Calculate the tile's dimensions for this element.
	tile.CalculateDimensions(GetTexture());

	TiledImageElementData* data = new TiledImageElementData;
	data->paint_area = paint_area;

	return reinterpret_cast<DecoratorDataHandle>(data);
}

void DecoratorTiledImage::ReleaseElementData(DecoratorDataHandle element_data) const
{
	delete reinterpret_cast<TiledImageElementData*>(element_data);
}

void DecoratorTiledImage::RenderElement(Element* element, DecoratorDataHandle element_data) const
{
	TiledImageElementData* data = reinterpret_cast<TiledImageElementData*>(element_data);
	float image_position_x = 0.f;
	if (const Property* property = element->GetProperty("image-position-x"))
		image_position_x = property->GetNumericValue().number;
	float image_position_y = 0.f;
	if (const Property* property = element->GetProperty("image-position-y"))
		image_position_y = property->GetNumericValue().number;

	if (data->image_position_x != image_position_x || data->image_position_y != image_position_y)
	{
		data->image_position_x = image_position_x;
		data->image_position_y = image_position_y;
		const RenderBox render_box = element->GetRenderBox(data->paint_area);
		const Vector2f offset = render_box.GetFillOffset();
		const Vector2f size = render_box.GetFillSize();
		const Vector2f natural_dimensions = tile.GetNaturalDimensions(element);

		Mesh mesh;
		tile.GenerateGeometry(mesh, element->GetComputedValues(), offset, size, natural_dimensions);

		if (tile.fit_mode == CONTAIN_HEIGHT_REPEAT_X && natural_dimensions.y > 0.f)
		{
			const float scaled_tile_width = natural_dimensions.x * size.y / natural_dimensions.y;
			if (scaled_tile_width > 0.f)
			{
				const float phase = std::fmod(-image_position_x / scaled_tile_width, 1.f);
				const int texture_axis = (tile.orientation == ROTATE_90 || tile.orientation == ROTATE_90_FLIP_HORIZONTAL) ? 1 : 0;
				for (Vertex& vertex : mesh.vertices)
					vertex.tex_coord[texture_axis] += phase;
			}
		}

		if (tile.fit_mode == CONTAIN_HEIGHT_REPEAT_X && size.y > 0.f)
		{
			const float phase = std::fmod(-image_position_y / size.y, 1.f);
			const int texture_axis = (tile.orientation == ROTATE_90 || tile.orientation == ROTATE_90_FLIP_HORIZONTAL) ? 0 : 1;
			for (Vertex& vertex : mesh.vertices)
				vertex.tex_coord[texture_axis] += phase;
		}

		data->geometry = element->GetRenderManager()->MakeGeometry(std::move(mesh));
	}

	data->geometry.Render(element->GetAbsoluteOffset(BoxArea::Border), GetTexture());
}

DecoratorTiledImageInstancer::DecoratorTiledImageInstancer() : DecoratorTiledInstancer(1)
{
	RegisterTileProperty("image", true);
	RegisterShorthand("decorator", "image", ShorthandType::RecursiveRepeat);
}

DecoratorTiledImageInstancer::~DecoratorTiledImageInstancer() {}

SharedPtr<Decorator> DecoratorTiledImageInstancer::InstanceDecorator(const String& /*name*/, const PropertyDictionary& properties,
	const DecoratorInstancerInterface& instancer_interface)
{
	DecoratorTiled::Tile tile;
	Texture texture;

	if (!GetTileProperties(&tile, &texture, 1, properties, instancer_interface))
		return nullptr;

	auto decorator = MakeShared<DecoratorTiledImage>();

	if (!decorator->Initialise(tile, texture))
		return nullptr;

	return decorator;
}

} // namespace Rml
