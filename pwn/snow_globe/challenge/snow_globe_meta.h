#ifndef SNOW_GLOBE_META
#define SNOW_GLOBE_META

#include <stdbool.h>

// env name, IPTC record, IPTC dataset, part_of_desc, template
#define USE_GLOBE_METADATA \
  X("IMG_TITLE", 2, 5, false, \
    "<dc:title><rdf:Alt><rdf:li>%s</rdf:li></rdf:Alt></dc:title>") \
  X("IMG_KEYWORDS", 2, 25, false, \
    "<dc:subject><rdf:Bag><rdf:li>%s</rdf:li></rdf:Bag></dc:subject>") \
  X("IMG_PROGRAM", 2, 65, true, \
    "xmp:CreatorTool=\"%s\"") \
  X("IMG_AUTHOR", 2, 80, false, \
    "<dc:creator><rdf:Seq><rdf:li>%s</rdf:li></rdf:Seq></dc:creator>") \
  X("IMG_AUTHOR_TITLE", 2, 85, true, \
    "photoshop:AuthorsPosition=\"%s\" ") \
  X("IMG_CITY", 2, 90, true, \
    "Iptc4xmpCore:CiAdrCity=\"%s\" ") \
  X("IMG_SUBLOCATION", 2, 92, true, \
    "Iptc4xmpExt:Sublocation=\"%s\" ") \
  X("IMG_PROVINCE", 2, 95, true, \
    "Iptc4xmpCore:CiAdrRegion=\"%s\" ") \
  X("IMG_COUNTRY", 2, 100, true, \
    "Iptc4xmpCore:CiAdrCtry=\"%s\" ") \
  X("IMG_COPYRIGHT", 2, 116, false, \
    "<dc:rights><rdf:Alt><rdf:li>%s</rdf:li></rdf:Alt></dc:rights>") \
  X("IMG_DESCRIPTION", 2, 120, false, \
    "<dc:description><rdf:Alt><rdf:li>%s</rdf:li></rdf:Alt></dc:description>") \
  X("IMG_WRITER", 2, 122, true, \
    "photoshop:CaptionWriter=\"%s\" ")


#endif /* SNOW_GLOBE_META */
