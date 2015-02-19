import _dk_core as core

SERIALIZE_XML = 0                   # XML with plain-text entity
SERIALIZE_ENCODED_CDATA_XML = 1     # XML with encoded cdata entity
SERIALIZE_BINARY = 2                # simple binary file (fastest)
SERIALIZE_COMPRESSED_BINARY = 3     # compressed binary file (smallest)

Resource = core.Resource
