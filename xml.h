/* XML */
/* ------------------------------------------------------------------------ */
enum XML_TextFormat
{
	/* Enum or all supported source text file formats */
	xml_text_format_ascii,
	xml_text_format_utf8,
	xml_text_format_utf16_be,
	xml_text_format_utf16_le,
	xml_text_format_utf32_be,
	xml_text_format_utf32_le,
};

enum XML_NodeType
{
	/* Enum for all XML node types parsed by the reader */
	xml_node_type_null,
	xml_node_type_element_start,
	xml_node_type_element_end,
	xml_node_type_text,
	xml_node_type_comment,
	xml_node_type_cdata,
	xml_node_type_unknown,
};

/* Reader */
/* ------------------------------------------------------------------------ */
struct XML_Attribute
{
	String name;
	String value;
};

struct XML_Reader
{
	String buffer;
	/* The string containing the data that we are going to read from */

	char *start;
	char *at;
	/* Pointers into "buffer"" */

	Size character_size;
	/* How many bytes is a character? */

	enum XML_NodeType current_node_type;

	enum XML_TextFormat source_format;
	enum XML_TextFormat target_format;
	/* Source and target formats for conversion, source is detected, target needs
	to be set somewhere before conversion happens */

	String node_name;
	/* Excerpt from buffer containing either the name of the node or the text inside */

	String empty_string;
	/* String that is empty, I have yet to find a use for this but it's in the original library */
	/* TODO: Maybe remove empty_string since we have no use for it */

	bool is_empty_element;
	/* If the text is all whitespace */

	char *special_characters;
	/* A list of characters escapes for use when converting text to target format */

	ARRAY_D(struct XML_Attribute) attributes;
	/* An array with the node attributes, name and value, cleared every time a new node appears */
};

String xml_replace_special_characters(struct XML_Reader *reader, String str);
bool xml_set_text(struct XML_Reader *reader, char *start, char *end);
void xml_push_attributes(struct XML_Reader *reader, struct XML_Attribute attribute);
void xml_clear_attributes(struct XML_Reader *reader);
void xml_parse_closing_element(struct XML_Reader *reader);
void xml_ignore_definition(struct XML_Reader *reader);
bool xml_parse_cdata(struct XML_Reader *reader);
void xml_parse_comment(struct XML_Reader *reader);
void xml_parse_opening_element(struct XML_Reader *reader);
void xml_parse_current_node(struct XML_Reader *reader);
bool xml_read(struct XML_Reader *reader);
void xml_read_file(struct XML_Reader *reader, File file);
struct XML_Reader * xml_reader_alloc();
void xml_reader_free(struct XML_Reader *reader);

/* Getters and Setters */
/* ------------------------------------------------------------------------ */
u32 xml_get_attribute_count(struct XML_Reader *reader);
String xml_get_attribute_name_by_id(struct XML_Reader *reader, u32 id);
String xml_get_attribute_name_by_value(struct XML_Reader *reader, String value);
String xml_get_attribute_value_by_id(struct XML_Reader *reader, u32 id);
String xml_get_attribute_value_by_name(struct XML_Reader *reader, String name);

enum XML_NodeType xml_get_node_type(struct XML_Reader *reader);
String xml_get_node_name(struct XML_Reader *reader);
String xml_get_node_data(struct XML_Reader *reader);

bool xml_is_empty_element(struct XML_Reader *reader);
