# Bf Optimizing Compiler
by Joshua Tlatelpa-Agustin 
10-12-24
######  Github link of project:
https://github.com/JoshAgustinT/hybrid_bf_compiler/tree/main
---

Testing methodology:
    Ran a makefile command that compiled the bf compiler, then each bf program in the benches folder was passed to the compiler, linked, and executed with the 'time ./a.out > /dev/null' command.

    Two runs, one with second order loop optimization pass OFF, and one with it ON.
---
RESULTS:

Output with second order loop optimization OFF:

    bench.b
    0.00user 0.00system 0:00.00elapsed 100%CPU (0avgtext+0avgdata 1340maxresident)k

    bottles.b
    0.00user 0.00system 0:00.00elapsed 89%CPU (0avgtext+0avgdata 1452maxresident)k
    
    deadcodetest.b
    0.00user 0.00system 0:00.00elapsed 87%CPU (0avgtext+0avgdata 1132maxresident)k

    hanoi.b
    0.38user 0.00system 0:00.38elapsed 100%CPU (0avgtext+0avgdata 1496maxresident)k

    hello.b
    0.00user 0.00system 0:00.00elapsed 87%CPU (0avgtext+0avgdata 1324maxresident)k

    long.b
    0.86user 0.00system 0:00.86elapsed 100%CPU (0avgtext+0avgdata 1336maxresident)k

    loopremove.b
    0.00user 0.00system 0:00.00elapsed 87%CPU (0avgtext+0avgdata 1364maxresident)k

    mandel.b
    0.78user 0.00system 0:00.78elapsed 100%CPU (0avgtext+0avgdata 1472maxresident)k

    serptri.b
    0.00user 0.00system 0:00.00elapsed 89%CPU (0avgtext+0avgdata 1364maxresident)k

    twinkle.b
    0.00user 0.00system 0:00.00elapsed 88%CPU (0avgtext+0avgdata 1460maxresident)k

---

Output with second order loop optimization ON:

    bench.b
    0.00user 0.00system 0:00.00elapsed 90%CPU (0avgtext+0avgdata 1432maxresident)k

    bottles.b
    0.00user 0.00system 0:00.00elapsed 89%CPU (0avgtext+0avgdata 1424maxresident)k

    deadcodetest.b
    0.00user 0.00system 0:00.00elapsed 86%CPU (0avgtext+0avgdata 1232maxresident)k

    hanoi.b
    0.01user 0.00system 0:00.01elapsed 100%CPU (0avgtext+0avgdata 1532maxresident)k

    hello.b
    0.00user 0.00system 0:00.00elapsed 91%CPU (0avgtext+0avgdata 1324maxresident)k

    long.b
    0.80user 0.00system 0:00.80elapsed 100%CPU (0avgtext+0avgdata 1432maxresident)k

    loopremove.b
    0.00user 0.00system 0:00.00elapsed 89%CPU (0avgtext+0avgdata 1440maxresident)k

    mandel.b
    0.75user 0.00system 0:00.75elapsed 100%CPU (0avgtext+0avgdata 1452maxresident)k

    serptri.b
    0.00user 0.00system 0:00.00elapsed 89%CPU (0avgtext+0avgdata 1340maxresident)k

    twinkle.b
    0.00user 0.00system 0:00.00elapsed 87%CPU (0avgtext+0avgdata 1352maxresident)k

---

We can see that the compiler was already pretty fast with our first loop optimization.

Most results were marginally faster with the second order loop optimization ie .75s vs .78s for mandel.b.
It may seem insignificant but it was consitently faster throughout the runs. It seems we got a pretty good run with low
interference to get .78s without the second order loop opt, because it's usually around .8-.9s. My guess would be that the program uses a lot of loops which are only executed a few times so the optimization didn't lead to big speedups.


However, hanoi.b got an insane speedup! My optimization pass made it ~38x faster! c:

I spent some time trying to stretch the kind of loops I could find closed forms for and found that many of the loops don't need to be restricted to the +-1 loop iterations. If we remove that restriction there's a bunch more loops we can replace with closed form. However, doing that extends compilation time a TON, since there's all these closed forms which are being computed and the final executable seems to be marginally slower. Probably the same issue as mandel.b, where the loops weren't being run enough times for them to make a difference. Some tests also stalled when doing this, presumably because a closed form wasn't computable and got stuck in an infinite loop trying to calculate it. So our second order loop definition remains the same as our simple loop definition except we also allow optimized simple loops inside it.

---

Closed forms of hanoi.b when second order loop MUST iterate +-1 

Compile time   =  0m6.124s

Execution time =  0m0.019s

    [>expr_simple:-0:-1,  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++expr_simple:-0:-1,  <-]
    closed_form:p0 = 0;p1 = 0;

    [>>expr_simple:-0:-1,  <expr_simple:-0:-1,1:1,     >expr_simple:--4:1,-1:1,0:-1,            <<-]
    closed_form:p0 = 0;p_2 = p0*p1 + p_2;p1 = p1;p2 = 0;

    [>>expr_simple:-0:-1,  <expr_simple:-0:-1,1:1,     >expr_simple:--4:1,-1:1,0:-1,            <<-]
    closed_form:p0 = 0;p_2 = p0*p1 + p_2;p1 = p1;p2 = 0;

---

Closed forms of hanoi.b when second order loop doesn't have an iteration offset restriction

Compile time =   1m3.088s

Execution time =  0m0.023s

output: (proof it works with all the closed forms c:)

![image](https://github.com/user-attachments/assets/1e1c3713-4a4f-4a8e-9f87-9822c9623c59)

All the simplified loops it optimizied from this point forward


            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--5:1,-3:1,0:-1,              <<expr_simple:--3:-1,0:-1,         >]
            closed_form:p_4 = -p0 - p_1 + p_2 + p_4;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [>expr_simple:-0:-1,  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++expr_simple:-0:-1,  <-]
            closed_form:p0 = 0;p1 = 0;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--5:1,0:-1,             <<<<<->>>>>]
            closed_form:p0 = 0;p_5 = p0 + p_5 - 1;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--5:1,-3:1,0:-1,              <<expr_simple:--3:-1,0:-1,         >]
            closed_form:p_4 = -p0 - p_1 + p_2 + p_4;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:-0:-1,  <<<++++++++++>>>]
            closed_form:p_3 = p_3 + 10;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  >expr_simple:-0:-1,  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<expr_simple:-0:-1,138:1,                                                                                                                                                                                                                                                                                       >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>expr_simple:-0:-1,  >>>>expr_simple:--138:1,-4:1,0:-1,                                                                                                                                                                                                                                                                                        <]
            closed_form:p_3 = p_137;p_137 = p_137;p1 = 0;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--6:1,-3:1,0:-1,                <<expr_simple:--4:-1,0:-1,           >]
            closed_form:p_5 = -p0 - p_1 + p_2 + p_5;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--5:1,-3:1,0:-1,              <<expr_simple:--3:-1,0:-1,         >]
            closed_form:p_4 = -p0 - p_1 + p_2 + p_4;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:-0:-1,  >expr_simple:-0:-1,  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<expr_simple:-0:-1,138:1,                                                                                                                                                                                                                                                                                       >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>expr_simple:-0:-1,  >>>expr_simple:--138:1,-3:1,0:-1,                                                                                                                                                                                                                                                                                        <]
            closed_form:p_2 = p_137;p_137 = p_137;p1 = 0;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--7:1,-3:1,0:-1,                  <<expr_simple:--5:-1,0:-1,             >]
            closed_form:p_6 = -p0 - p_1 + p_2 + p_6;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--7:1,-3:1,0:-1,                  <<expr_simple:--5:-1,0:-1,             >]
            closed_form:p_6 = -p0 - p_1 + p_2 + p_6;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<<->>>]
            closed_form:p0 = 0;p_3 = p0 + p_3 - 1;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--5:1,-3:1,0:-1,              <<expr_simple:--3:-1,0:-1,         >]
            closed_form:p_4 = -p0 - p_1 + p_2 + p_4;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--5:1,-3:1,0:-1,              <<expr_simple:--3:-1,0:-1,         >]
            closed_form:p_4 = -p0 - p_1 + p_2 + p_4;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--7:1,-3:1,0:-1,                  <<expr_simple:--5:-1,0:-1,             >]
            closed_form:p_6 = -p0 - p_1 + p_2 + p_6;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--6:1,-3:1,0:-1,                <<expr_simple:--4:-1,0:-1,           >]
            closed_form:p_5 = -p0 - p_1 + p_2 + p_5;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<+++++>>>>]
            closed_form:p_4 = p_4 + 5;p0 = 0;

            [expr_simple:--2:1,0:-1,       <<->>]
            closed_form:p0 = 0;p_2 = p0 + p_2 - 1;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--6:1,-3:1,0:-1,                <<expr_simple:--4:-1,0:-1,           >]
            closed_form:p_5 = -p0 - p_1 + p_2 + p_5;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--7:1,-3:1,0:-1,                  <<expr_simple:--5:-1,0:-1,             >]
            closed_form:p_6 = -p0 - p_1 + p_2 + p_6;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:-0:-1,  >expr_simple:-0:-1,  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<expr_simple:-0:-1,56:1,                                                                                                                   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>expr_simple:-0:-1,  >>>expr_simple:--56:1,-3:1,0:-1,                                                                                                                    <]
            closed_form:p_2 = p_55;p1 = 0;p_55 = p_55;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [>>expr_simple:-0:-1,  <expr_simple:-0:-1,1:1,     >expr_simple:--4:1,-1:1,0:-1,            <<-]
            closed_form:p0 = 0;p_2 = p0*p1 + p_2;p1 = p1;p2 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  >expr_simple:-0:-1,  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<expr_simple:-0:-1,56:1,                                                                                                                   >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>expr_simple:-0:-1,  >>>>expr_simple:--56:1,-4:1,0:-1,                                                                                                                    <]
            closed_form:p_3 = p_55;p1 = 0;p_55 = p_55;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<->>]
            closed_form:p0 = 0;p_2 = p0 + p_2 - 1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--5:1,0:-1,             <+++++++++++++++++++++++++++++++++++++++++++>]
            closed_form:p_1 = p_1 + 43;p0 = 0;p_5 = p0 + p_5;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  >expr_simple:-0:-1,  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<expr_simple:-0:-1,97:1,                                                                                                                                                                                                     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>expr_simple:-0:-1,  >>>expr_simple:--97:1,-3:1,0:-1,                                                                                                                                                                                                      <]
            closed_form:p_2 = p_96;p_96 = p_96;p1 = 0;p0 = 0;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:-0:-1,  <<<<+++++>>>>]
            closed_form:p_4 = p_4 + 5;p0 = 0;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<++++++++++>>>]
            closed_form:p_3 = p_3 + 10;p0 = 0;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--5:1,-3:1,0:-1,              <<expr_simple:--3:-1,0:-1,         >]
            closed_form:p_4 = -p0 - p_1 + p_2 + p_4;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:-0:-1,  <<<expr_simple:-0:-1,  +>expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p_3 = 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<----->>>>]
            closed_form:p_4 = p_4 - 5;p0 = 0;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--6:1,-3:1,0:-1,                <<expr_simple:--4:-1,0:-1,           >]
            closed_form:p_5 = -p0 - p_1 + p_2 + p_5;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_2 = p0 + p_2;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--7:1,-3:1,0:-1,                  <<expr_simple:--5:-1,0:-1,             >]
            closed_form:p_6 = -p0 - p_1 + p_2 + p_6;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--2:1,0:-1,       <<->>]
            closed_form:p0 = 0;p_2 = p0 + p_2 - 1;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<expr_simple:-0:-1,  +>>expr_simple:-0:-1,  +>]
            closed_form:p_1 = 1;p_3 = 1;p0 = 0;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--6:1,-3:1,0:-1,                <<expr_simple:--4:-1,0:-1,           >]
            closed_form:p_5 = -p0 - p_1 + p_2 + p_5;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <<->>]
            closed_form:p0 = 0;p_2 = p0 + p_2 - 1;

            [expr_simple:-0:-1,  >expr_simple:-0:-1,  <<<<<<<<<<<<<<<<expr_simple:-0:-1,16:1,                                   >>>>>>>>>>>expr_simple:-0:-1,  >>>>>expr_simple:--16:1,-5:1,0:-1,                                    expr_simple:-0:-1,  <<<<<<<expr_simple:-0:-1,7:1,                 >>>expr_simple:-0:-1,  >>>>expr_simple:--7:1,-4:1,0:-1,                  <<<expr_simple:-0:-1,  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++>>]
            closed_form:p_2 = 88;p_3 = p_6;p_6 = p_6;p_4 = p_15;p_15 = p_15;p1 = 0;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<----->>>>]
            closed_form:p_4 = p_4 - 5;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [>>expr_simple:-0:-1,  <expr_simple:-0:-1,1:1,     >expr_simple:--4:1,-1:1,0:-1,            <<-]
            closed_form:p0 = 0;p_2 = p0*p1 + p_2;p1 = p1;p2 = 0;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  +>>>]
            closed_form:p_3 = 1;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  >expr_simple:-0:-1,  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<expr_simple:-0:-1,97:1,                                                                                                                                                                                                     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>expr_simple:-0:-1,  >>>>expr_simple:--97:1,-4:1,0:-1,                                                                                                                                                                                                      <]
            closed_form:p_3 = p_96;p_96 = p_96;p1 = 0;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:--1:1,0:-1,     <<<expr_simple:-0:-1,  >>>]
            closed_form:p_3 = 0;p0 = 0;p_1 = p0 + p_1;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = p_7 + 1;p0 = 0;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--7:1,-3:1,0:-1,                  <<expr_simple:--5:-1,0:-1,             >]
            closed_form:p_6 = -p0 - p_1 + p_2 + p_6;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:-0:-1,  <<<<<<<+>>>>>>>]
            closed_form:p_7 = -38281503*c5*p0**2/88302869 + c5*p0*p_7 - 2058449380*c5*p0/88302869 - 6222646*c5*p_7**2/88302869 + 19975281352*c5/88302869 + p_7*(1 - 586787976*c5/88302869) + 1;p0 = -38281503*c5*p0**2/88302869 + c5*p0*p_7 - 2058449380*c5*p0/88302869 - 6222646*c5*p_7**2/88302869 - 586787976*c5*p_7/88302869 + 19975281352*c5/88302869;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;

            [expr_simple:--1:1,0:-1,     >expr_simple:-0:-1,  <<<expr_simple:-0:-1,3:1,         >>>expr_simple:--6:1,-3:1,0:-1,                <<expr_simple:--4:-1,0:-1,           >]
            closed_form:p_5 = -p0 - p_1 + p_2 + p_5;p1 = 0;p0 = 0;p_2 = p_2;p_1 = 0;

            [expr_simple:--2:1,0:-1,       <expr_simple:-0:-1,  >]
            closed_form:p0 = 0;p_1 = 0;p_2 = p0 + p_2;

            [expr_simple:--3:1,0:-1,         <<expr_simple:-0:-1,  +>>]
            closed_form:p_2 = 1;p0 = 0;p_3 = p0 + p_3;
