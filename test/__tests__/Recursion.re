open Jest;
open TestUtils;

[@decco]
type basic = Rec(basic) | End;

describe("basic", () => {
    testEncode("encode", Rec(Rec(End)), basic_encode, {|["Rec",["Rec",["End"]]]|});
});
