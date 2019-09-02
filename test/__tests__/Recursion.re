open Jest;
open TestUtils;

[@decco] type basic = Basic(basic) | End;
[@decco] type nested = Nested(option(nested)) | End;
[@decco] type tuple = Tuple((int, tuple)) | End;

describe("basic", () => {
    let v = Basic(Basic(End));
    let jsonStr = {|["Basic",["Basic",["End"]]]|};

    testEncode("encode", v, basic_encode, jsonStr);
    testGoodDecode("decode", basic_decode, Js.Json.parseExn(jsonStr), v);
});

describe("nested", () => {
    let v = Nested(Some(Nested(None)));
    let jsonStr = {|["Nested",["Nested",null]]|};

    testEncode("encode", v, nested_encode, jsonStr);
    testGoodDecode("decode", nested_decode, Js.Json.parseExn(jsonStr), v);
});

describe("tuple", () => {
    let v = Tuple((0, End));
    let jsonStr = {|["Tuple",[0,["End"]]]|};

    testEncode("encode", v, tuple_encode, jsonStr);
    testGoodDecode("decode", tuple_decode, Js.Json.parseExn(jsonStr), v);
});

[@decco] type nonRec = int;
module NonRec = {
    [@decco] type nonrec nonRec = NonRec(nonRec);
    describe("nonrec", () => {
        let v = NonRec(5);
        let jsonStr = {|["NonRec",5]|};

        testEncode("encode", v, nonRec_encode, jsonStr);
        testGoodDecode("decode", nonRec_decode, Js.Json.parseExn(jsonStr), v);
    });
};
