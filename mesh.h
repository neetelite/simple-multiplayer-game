#define FACE_INDICES_MAX 0x7fff
#define BONE_WEIGHT_MAX 0x7fffffff
#define MESH_VERTICE_MAX 0x7fffffff
#define MESH_FACE_MAX 0x7fffffff
#define MESH_COLOR_SET_MAX 0x8
#define MESH_TEX_COORD_MAX 0x8

struct Face
{
	ARRAY_D(Index) indices;
};

struct WeightVertex
{
	u32 verted_id;
	f32 weight;
};

struct Bone
{
	String name;

	u32 weight_count;
	struct WeightVertex *weights;
	/* TODO: Array? */

	mat4 offset_matrix;
};

enum PrimitiveType
{
	primitive_type_point = 0x1,
	primitive_type_line = 0x2,
	primitive_type_triangle = 0x4,
	primitive_type_polygon = 0x8,
};

struct AnimMesh
{
	String name;

	v3 vertices;
	v3 normals;
	v3 tangents;
	v3 bitangents;
	v4 colors[MESH_COLOR_SET_MAX];
	v3 tex_coords[MESH_TEX_COORD_MAX];

	u32 vertice_count;
	f32 weight;
};

enum MorphingMethod
{
	morphing_method_vertex_blend,
	morphing_method_normalized,
	morphing_method_relative,
};

struct Mesh
{
	u32 position_count;
	v3  *positions;
	u32 *indices;

	v4 color;

	mat4 transform;

	u32 VAO, VBO;
};
