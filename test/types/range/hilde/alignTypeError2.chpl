var i8  = 3:int(8);
var i16 = 3:int(16);
var i32 = 3:int(32);
var i64 = 3:int(64);
var i   = 3:int;
var u8  = 3:uint(8);
var u16 = 3:uint(16);
var u32 = 3:uint(32);
var u64 = 3:uint(64);
var u   = 3:uint;

writeln((i8.. by 10 align i16).type:string);
writeln((i8.. by 10 align i32).type:string);
writeln((i8.. by 10 align u8).type:string);
writeln((i8.. by 10 align u16).type:string);
writeln((i16.. by 10 align i32).type:string);
writeln((i16.. by 10 align u16).type:string);
writeln((u8.. by 10 align i8).type:string);
writeln((u8.. by 10 align i16).type:string);
writeln((u8.. by 10 align i32).type:string);
writeln((u8.. by 10 align u16).type:string);
writeln((u16.. by 10 align i8).type:string);
writeln((u16.. by 10 align i16).type:string);
writeln((u16.. by 10 align i32).type:string);
