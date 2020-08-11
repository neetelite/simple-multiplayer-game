String
xml_replace_special_characters(struct XML_Reader *reader, String str)
{
	/* TODO */
	String result = {0};
	result = str;
	return(result);
}

bool
xml_set_text(struct XML_Reader *reader, char *start, char *end)
{
	/* Check if text is more than 2 character, and if not, check if there is
	   only white space, so that this text won't be reported */
	if(end - start < 3)
	{
		char *at = start;
		for(; at != end; ++at) if(!char_is_whitespace(*at)) break;

		if(at == end) return(false);
	}

	/* Set current text to the parsed text, and replace xml special characters */
	String str = str_excerpt(reader->buffer, start, end);
	reader->node_name = xml_replace_special_characters(reader, str);

	reader->current_node_type = xml_node_type_text;

	return(true);
}

void
xml_push_attributes(struct XML_Reader *reader, struct XML_Attribute attribute)
{
	array_push(&reader->attributes, attribute);


	#if 0

	#endif
}

void
xml_clear_attributes(struct XML_Reader *reader)
{
	array_clear(&reader->attributes);
}

void
xml_parse_closing_element(struct XML_Reader *reader)
{
	reader->current_node_type = xml_node_type_element_end;
	reader->is_empty_element = false;
	xml_clear_attributes(reader);

	++reader->at;
	char *begin_close = reader->at;

	while(*reader->at != L'>') ++reader->at;

	reader->node_name = str_excerpt(reader->buffer, begin_close, reader->at);
	++reader->at;
}

void
xml_ignore_definition(struct XML_Reader *reader)
{
	reader->current_node_type = xml_node_type_unknown;

	while(*reader->at != L'>') ++reader->at;
	++reader->at;
}

bool
xml_parse_cdata(struct XML_Reader *reader)
{
	if(*(reader->at + 1) != L'[') return(false);

	reader->current_node_type = xml_node_type_cdata;

	/* Skip '<![CDATA[' */
	i32 count = 0;
	while(*reader->at && count < 8)
	{
		++reader->at;
		++count;
	}

	if(!*reader->at) return(true);

	char *cdata_begin = reader->at;
	char *cdata_end = NULL;

	/* Find end of CDATA */
	while(*reader->at && !cdata_end)
	{
		if(*reader->at == L'>' &&
		   (*(reader->at - 1) == ']') &&
		   (*(reader->at - 2) == ']'))
		{
			cdata_end = reader->at - 2;
		}
		++reader->at;
	}

	if(cdata_end) reader->node_name = str_excerpt(reader->buffer, cdata_begin, cdata_end);
	else reader->node_name = STR_NULL;

	return(true);
}

void
xml_parse_comment(struct XML_Reader *reader)
{
	reader->current_node_type = xml_node_type_comment;
	++reader->at;

	char *comment_begin = reader->at;
	i32 count = 1;

	while(count)
	{
		if(*reader->at == L'>') --count;
		else if(*reader->at == L'<') ++count;
		++reader->at;
	}

	reader->at -= 3;
	reader->node_name = str_excerpt(reader->buffer, comment_begin + 2, reader->at - 2);
	reader->at += 3;
}

void
xml_parse_opening_element(struct XML_Reader *reader)
{
	reader->current_node_type = xml_node_type_element_start;
	reader->is_empty_element = false;
	xml_clear_attributes(reader);

	/* Find name */
	char *start_name = reader->at;

	while(*reader->at != L'>' && !char_is_whitespace(*reader->at)) ++reader->at;
	char *end_name = reader->at;

	/* Find Attributes */
	while(*reader->at != L'>')
	{
		if(char_is_whitespace(*reader->at)) ++reader->at;
		else
		{
			if(*reader->at != L'/')
			{
				/* We've got the attributes */
				/* Read their names */
				char *attribute_name_begin = reader->at;

				while(!char_is_whitespace(*reader->at) && *reader->at != L'=') ++reader->at;

				char *attribute_name_end = reader->at;
				++reader->at;

				/* Read the attribute value */
				while((*reader->at != L'\"') && (*reader->at != L'\'') && *reader->at) ++reader->at;

				/* Malformed XML file */
				if(!*reader->at) return;

				char attribute_quote_char = *reader->at;

				++reader->at;
				char *attribute_value_begin = reader->at;

				while(*reader->at != attribute_quote_char && *reader->at) ++reader->at;
				/* Malformed XML file */
				if(!*reader->at) return;

				char *attribute_value_end = reader->at;
				++reader->at;

				struct XML_Attribute attribute;
				attribute.name = str_excerpt(reader->buffer, attribute_name_begin,
							     attribute_name_end);

				#if 0
				String str = str_excerpt(attribute_value_begin, attribute_value_end);
				attribute.value = xml_replace_special_characters(reader, str);
				#else
				attribute.value  = str_excerpt(reader->buffer, attribute_value_begin,
							       attribute_value_end);
				#endif

				xml_push_attributes(reader, attribute);
			}
			else
			{
				/* Tag is closed directly */
				++reader->at;
				reader->is_empty_element = true;
				break;
			}
		}
	}

	/* Check if this tag is closing directly */
	if(end_name > start_name && *(end_name - 1 ) == L'/')
	{
		/* Directly closing tag */
		reader->is_empty_element = true;
		--end_name;
	}

	reader->node_name = str_excerpt(reader->buffer, start_name, end_name);
	++reader->at;
}

void
xml_parse_current_node(struct XML_Reader *reader)
{
	char *start = reader->at;

	/* Move forward until '<' is found */
	while(*reader->at != L'<' && *reader->at) ++reader->at;

	if(!*reader->at) return;
	if(reader->at - start > 0)
	{
		/* We found some text, store it */
		if(xml_set_text(reader, start, reader->at)) return;
	}
	++reader->at;

	/* Based on current token, parse and report next element */
	switch(*reader->at)
	{
	case L'/':
	{
		xml_parse_closing_element(reader);
	} break;
	case L'?':
	{
		xml_ignore_definition(reader);
	} break;
	case L'!':
	{
		if(!xml_parse_cdata(reader)) xml_parse_comment(reader);
	} break;
	default:
	{
		xml_parse_opening_element(reader);
	} break;
	}
}

bool
xml_read(struct XML_Reader *reader)
{
	bool result = false;

	if(reader->at && (u32)(reader->at - reader->start) < reader->buffer.len - 1 && *reader->at != 0)
	{
		xml_parse_current_node(reader);
		result = true;
	}

	return(result);
}

void
xml_convert_text_data(struct XML_Reader *reader)
{

	/* TODO */
	/* Convert from litle to big if necessary */
	#if 0
	if(reader->source_format == xml_text_format_utf16_le ||
	   reader->source_format == xml_text_format_utf63_le)
	{
		xml_
	}
	#endif

	/* TODO */
	/* Check if conversion is necessary */
}

void
xml_read_file(struct XML_Reader *reader, File file)
{
	ASSERT(file.open);

	/* We need 32 bits of zero at the end */
	i32 size = file_get_size(file);
	size += 4;

	/* Allocate file data */
	reader->buffer = str_alloc_n(NULL, size);
	file_read(file, reader->buffer.data, size);

	/* The zeroes we promissed */
	reader->buffer.data[size - 1] = 0;
	reader->buffer.data[size - 2] = 0;
	reader->buffer.data[size - 3] = 0;
	reader->buffer.data[size - 4] = 0;

	/* TODO LEARN: Reinterpret_cast */
	u8 *data8 = (u8 *)reader->buffer.data;
	u16 *data16 = (u16 *)data8;
	u32 *data32 = (u32 *)data8;

	/* Now we need to convert the data to the desired target format
	   based on the byte order mark */
	const unsigned char UTF8[] = {0xEF, 0xBB, 0xBF}; // 0xEFBBBF;
	const int UTF16_BE = 0xFFFE;
	const int UTF16_LE = 0xFEFF;
	const int UTF32_BE = 0xFFFE0000;
	const int UTF32_LE = 0x0000FEFF;

	/* Check source for all UTF versions and convert to target data format */
	if(size >= 4 && data32[0] == (u32)UTF32_BE)
	{
		/* UTF-32, big endian */
		reader->source_format = xml_text_format_utf32_be;
		reader->start = reader->buffer.data + 4;
		reader->character_size = 4;
	}
	else if(size >= 4 && data32[0] == (u32)UTF32_LE)
	{
		/* UTF-32, little endian */
		reader->source_format = xml_text_format_utf32_le;
		reader->start = reader->buffer.data + 4;
		reader->character_size = 4;
	}
	else if(size >= 2 && data16[0] == UTF16_BE)
	{
		/* UTF-16, big endian */
		reader->source_format = xml_text_format_utf16_be;
		reader->start = reader->buffer.data + 2;
		reader->character_size = 2;
	}
	else if(size >= 2 && data16[0] == UTF16_LE)
	{
		/* UTF-16, little endian */
		reader->source_format = xml_text_format_utf16_le;
		reader->start = reader->buffer.data + 2;
		reader->character_size = 2;
	}
	else if(size >= 3 && data8[0] == UTF8[0] && data8[1] == UTF8[1] && data8[2] == UTF8[2])
	{
		/* UTF-8 */
		reader->source_format = xml_text_format_utf8;
		reader->start = reader->buffer.data + 3;
		reader->character_size = 1;
	}
	else{
		/* ASCII */
		reader->source_format = xml_text_format_ascii;
		reader->character_size = 1;
		reader->start = reader->buffer.data;
	}
	xml_convert_text_data(reader);

	reader->at = reader->start;
}

struct XML_Reader *
xml_reader_alloc()
{
	struct XML_Reader *result = NULL;
	result = mem_alloc(sizeof(struct XML_Reader), true);
	array_d_init(&result->attributes, 1);

	return(result);
}

void
xml_reader_free(struct XML_Reader *reader)
{
	array_term(&reader->attributes);

	str_free(&reader->buffer);
	/* TODO: Are there more buffers that need to be free */

	mem_free(reader);
}

/* Getters and Setters */
/* ------------------------------------------------------------------------ */
enum XML_NodeType
xml_get_node_type(struct XML_Reader *reader)
{
	return(reader->current_node_type);
}

u32
xml_get_attribute_count(struct XML_Reader *reader)
{
	return(reader->attributes.count);
}

String
xml_get_attribute_name_by_id(struct XML_Reader *reader, u32 id)
{
	String result = STR_NULL;
	if(id > xml_get_attribute_count(reader)) return(result);

	struct XML_Attribute *attribute = &reader->attributes.data[id];

	result = attribute->name;
	return(result);
}

String
xml_get_attribute_name_by_value(struct XML_Reader *reader, String value)
{
	String result = {0};
	for(i32 i = 0; i < reader->attributes.count; ++i)
	{
		struct XML_Attribute *attribute = &reader->attributes.data[i];
		if(str_eql(attribute->value, value))
		{
			result = attribute->name;
			break;
		}
	}
	return(result);
}

String
xml_get_attribute_value_by_id(struct XML_Reader *reader, u32 id)
{
	String result = STR_NULL;
	if(id > xml_get_attribute_count(reader)) return(result);

	struct XML_Attribute *attribute = &reader->attributes.data[id];

	result = attribute->value;
	return(result);
}

String
xml_get_attribute_value_by_name(struct XML_Reader *reader, String name)
{
	String result = {0};
	for(i32 i = 0; i < reader->attributes.count; ++i)
	{
		struct XML_Attribute *attribute = &reader->attributes.data[i];
		if(str_eql(attribute->name, name))
		{
			result = attribute->value;
			break;
		}
	}

	return(result);
}

String
xml_get_node_name(struct XML_Reader *reader)
{
	return(reader->node_name);
}

String
xml_get_node_data(struct XML_Reader *reader)
{
	return(reader->node_name);
}


bool
xml_is_empty_element(struct XML_Reader *reader)
{
	bool result = false;
	result = reader->is_empty_element;
	return(result);
}
