template <class T>
struct C {
  T x;
  void bar();
};

template <class T>
void foo() {
  C<T> d;
  d.x;  // spelling range is empty, use cursor extent for range
  d.bar();  // spelling range is empty, use cursor extent for range

  auto e = new C<T>;
  e->x;  // `x` seems not exposed by libclang
  e->bar();  // `bar` seems not exposed by libclang
}

/*
EXTRA_FLAGS:
-fms-extensions
-fms-compatibility
-fdelayed-template-parsing

OUTPUT:
{
  "includes": [],
  "skipped_by_preprocessor": [],
  "types": [{
      "id": 0,
      "usr": "c:@ST>1#T@C",
      "short_name": "C",
      "detailed_name": "C",
      "kind": 7,
      "definition_spelling": "2:8-2:9",
      "definition_extent": "2:1-5:2",
      "parents": [],
      "derived": [],
      "types": [],
      "funcs": [0],
      "vars": [0],
      "instances": [],
      "uses": ["2:8-2:9"]
    }, {
      "id": 1,
      "usr": "c:member_ref_in_template.cc@10",
      "short_name": "T",
      "detailed_name": "T",
      "kind": 0,
      "definition_spelling": "1:17-1:18",
      "definition_extent": "1:11-1:18",
      "parents": [],
      "derived": [],
      "types": [],
      "funcs": [],
      "vars": [],
      "instances": [],
      "uses": ["1:17-1:18", "3:3-3:4"]
    }],
  "funcs": [{
      "id": 0,
      "is_operator": false,
      "usr": "c:@ST>1#T@C@F@bar#",
      "short_name": "bar",
      "detailed_name": "void C::bar()",
      "kind": 16,
      "declarations": [{
          "spelling": "4:8-4:11",
          "extent": "4:3-4:13",
          "content": "void bar()",
          "param_spellings": []
        }],
      "declaring_type": 0,
      "base": [],
      "derived": [],
      "locals": [],
      "callers": [],
      "callees": []
    }, {
      "id": 1,
      "is_operator": false,
      "usr": "c:@FT@>1#Tfoo#v#",
      "short_name": "foo",
      "detailed_name": "void foo()",
      "kind": 12,
      "declarations": [],
      "definition_spelling": "8:6-8:9",
      "definition_extent": "8:1-8:11",
      "base": [],
      "derived": [],
      "locals": [],
      "callers": [],
      "callees": []
    }],
  "vars": [{
      "id": 0,
      "usr": "c:@ST>1#T@C@FI@x",
      "short_name": "x",
      "detailed_name": "T C::x",
      "definition_spelling": "3:5-3:6",
      "definition_extent": "3:3-3:6",
      "declaring_type": 0,
      "kind": 14,
      "uses": ["3:5-3:6"]
    }]
}
*/
