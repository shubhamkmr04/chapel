use BigInteger;

// Tests the division functions
var a = new bigint(  8);
var b = new bigint( 10);
var c = new bigint(-27);
var d = new bigint();

d.div_q(c, a, round.up);
b.div_r(c, a, round.up);
writeln(d, " ", b);

d.div_q(c, a, round.down);
b.div_r(c, a, round.down);
writeln(d, " ", b);

d.div_q(c, a, round.zero); // same as up   for negative integers
b.div_r(c, a, round.zero); // same as down for positive integers
writeln(d, " ", b);

c.neg(c);
d.div_qr(b, c, a, round.up);
writeln(d, " ", b);

d.div_qr(b, c, a, round.down);
writeln(d, " ", b);

d.div_qr(b, c, a, round.zero); // same as down for positive integers
writeln(d, " ", b);

writeln();

c.neg(c);
d.div_q(c, 8, round.up);
b.div_r(c, 8, round.up);
writeln(d, " ", b);

d.div_q(c, 8, round.down);
b.div_r(c, 8, round.down);
writeln(d, " ", b);

d.div_q(c, 8, round.zero); // same as down for positive integers
b.div_r(c, 8, round.zero); // same as down for positive integers
writeln(d, " ", b);

c.neg(c);
d.div_qr(b, c, 8, round.up);
writeln(d, " ", b);

d.div_qr(b, c, 8, round.down);
writeln(d, " ", b);

d.div_qr(b, c, 8, round.zero); // same as down for positive integers
writeln(d, " ", b);

c.neg(c);
writeln();


// q = (n / 2^d)
d.div_q_2exp(c, 3, round.up);
b.div_r_2exp(c, 3, round.up);
writeln(d, " ", b);

d.div_q_2exp(c, 3, round.down);
b.div_r_2exp(c, 3, round.down);
writeln(d, " ", b);

d.div_q_2exp(c, 3, round.zero);
b.div_r_2exp(c, 3, round.zero);
writeln(d, " ", b);

writeln();

d.mod(c, a);
writeln(d);

var d2 : uint = 0;

d2 = d.mod(c, 8);
writeln(d2);

b.set(16);
d.divexact(b, a);
writeln(d);

d.divexact(b, 4);
writeln(d);

var d3 : int = b.divisible_p(a);
writeln(d3);

d3 = b.divisible_p(3);
writeln(d3, " ");
writeln(b.divisible_2exp_p(3)); // divisble by 2^3

a.set(25);
b.set(20);
c.set(5);

d3 = a.congruent_p(c, b); // 25 congruent to 5 mod 20
writeln(d3);

d3 = a.congruent_p(10, 20); //25 not congruent to 10 mod 20
writeln(d3);

a.set(45);
d3 = a.congruent_2exp_p(c, 3); //45 congruent to 5 mod 8
writeln(d3);
