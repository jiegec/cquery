struct T {};

extern T t;
/*
OUTPUT:
{
  "includes": [],
  "skipped_by_preprocessor": [],
  "types": [{
      "id": 0,
      "usr": "c:@S@T",
      "short_name": "T",
      "detailed_name": "T",
      "kind": 6,
      "definition_spelling": "1:8-1:9",
      "definition_extent": "1:1-1:12",
      "parents": [],
      "derived": [],
      "types": [],
      "funcs": [],
      "vars": [],
      "instances": [0],
      "uses": ["1:8-1:9", "3:8-3:9"]
    }],
  "funcs": [],
  "vars": [{
      "id": 0,
      "usr": "c:@t",
      "short_name": "t",
      "detailed_name": "T t",
      "declaration": "3:10-3:11",
      "variable_type": 0,
      "kind": 13,
      "uses": ["3:10-3:11"]
    }]
}
*/
