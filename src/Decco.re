open Belt.Result;

type abc = Blah({a:string});

type decodeError = {
    path: string,
    message: string,
    value: Js.Json.t
};

type result('a) = Belt.Result.t('a, decodeError);
type decoder('a) = Js.Json.t => result('a);
type encoder('a) = 'a => Js.Json.t;
type codec('a) = (encoder('a), decoder('a));

let error = (~path=?, message, value) =>{
    let path = switch path {
        | None => ""
        | Some(s) => s
    };
    Error({ path, message, value });
};

let stringToJson = (s) => Js.Json.string(s);
let stringFromJson = (j) =>
    switch (Js.Json.classify(j)) {
        | Js.Json.JSONString(s) => Ok(s)
        | _ => Error({ path: "", message: "Not a string", value: j })
    };

let intToJson = (i) => i |> float_of_int |> Js.Json.number;
let intFromJson = (j) =>
    switch (Js.Json.classify(j)) {
        | Js.Json.JSONNumber(f) =>
            (float_of_int(Js.Math.floor(f)) == f) ?
                Ok(Js.Math.floor(f))
            : Error({ path: "", message: "Not an integer", value: j })

        | _ => Error({ path: "", message: "Not a number", value: j })
    };

let int64ToJson = (i) => i
    |> Int64.float_of_bits
    |> Js.Json.number;

let int64FromJson = (j) =>
    switch (Js.Json.classify(j)) {
        | Js.Json.JSONNumber(n) => Ok(Int64.bits_of_float(n))
        | _ => error("Not a number", j)
    };

let int64ToJsonUnsafe = (i) => i
    |> Int64.to_float
    |> Js.Json.number;

let int64FromJsonUnsafe = (j) =>
    switch (Js.Json.classify(j)) {
        | Js.Json.JSONNumber(n) => Ok(Int64.of_float(n))
        | _ => error("Not a number", j)
    };

let floatToJson = (v) => v |> Js.Json.number;
let floatFromJson = (j) =>
    switch (Js.Json.classify(j)) {
        | Js.Json.JSONNumber(f) => Ok(f)
        | _ => Error({ path: "", message: "Not a number", value: j })
    };

let boolToJson = (v) => v |> Js.Json.boolean;
let boolFromJson = (j) =>
    switch (Js.Json.classify(j)) {
        | Js.Json.JSONTrue => Ok(true)
        | Js.Json.JSONFalse => Ok(false)
        | _ => Error({ path: "", message: "Not a boolean", value: j })
    };

let unitToJson = () => Js.Json.number(0.0);
let unitFromJson = (_) => Ok(());

let arrayToJson = (encoder, arr) =>
    arr
    |> Js.Array.map(encoder)
    |> Js.Json.array;

let arrayFromJson = (decoder, json) =>
    switch (Js.Json.classify(json)) {
        | Js.Json.JSONArray(arr) =>
            Js.Array.reducei((acc, jsonI, i) => {
                switch (acc, decoder(jsonI)) {
                    | (Error(_), _) => acc

                    | (_, Error({ path } as error)) =>
                        Error({ ...error, path: "[" ++ string_of_int(i) ++ "]" ++ path })

                    | (Ok(prev), Ok(newVal)) => Ok(Js.Array.concat([|newVal|], prev))
                };
            }, Ok([||]), arr)

        | _ => Error({ path: "", message: "Not an array", value: json })
    };

let listToJson = (encoder, list) =>
    list
    |> Array.of_list
    |> arrayToJson(encoder);

let listFromJson = (decoder, json) =>
    json
    |> arrayFromJson(decoder)
    |> map(_, Array.to_list);

let optionToJson = (encoder, opt) =>
    switch opt {
        | Some(x) => encoder(x)
        | None => Js.Json.null
    };

let optionFromJson = (decoder, json) =>
    switch (Js.Json.classify(json)) {
        | Js.Json.JSONNull => Ok(None)
        | _ => decoder(json) |> map(_, v => Some(v))
    };

let resultToJson = (okEncoder, errorEncoder, result) =>
    switch result {
        | Ok(v) => [| Js.Json.string("Ok"), okEncoder(v) |]
        | Error(e) => [| Js.Json.string("Error"), errorEncoder(e) |]
    }
    |> Js.Json.array;

let resultFromJson = (okDecoder, errorDecoder, json) =>
    switch (Js.Json.decodeArray(json)) {
        | Some([| variantConstructorId, payload |]) =>
            switch (Js.Json.decodeString(variantConstructorId)) {
                | Some("Ok") =>
                    okDecoder(payload)
                    -> Belt.Result.map(v => Ok(v))

                | Some("Error") =>
                    switch(errorDecoder(payload)) {
                        | Ok(v) => Ok(Error(v))
                        | Error(e) => Error(e)
                    }

                | Some(_) => error("Expected either \"Ok\" or \"Error\"", variantConstructorId)
                | None => error("Not a string", variantConstructorId)
            }
        | Some(_) => error("Expected exactly 2 values in array", json)
        | None => error("Not an array", json)
    };

module Codecs {
    include Codecs;
    let string = (stringToJson, stringFromJson);
    let int = (intToJson, intFromJson);
    let int64Unsafe = (int64ToJsonUnsafe, int64FromJsonUnsafe);
    let float = (floatToJson, floatFromJson);
    let bool = (boolToJson, boolFromJson);
    let array = (arrayToJson, arrayFromJson);
    let list = (listToJson, listFromJson);
    let option = (optionToJson, optionFromJson);
    let unit = (unitToJson, unitFromJson);
};
