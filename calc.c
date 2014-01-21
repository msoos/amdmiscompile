void memcpyN(unsigned* to, unsigned* from, unsigned num)
{
    for(int i = num-1; i >= 0; i--) {
        to[i] = from[i];
    }
}

unsigned calc_output(struct Calc* calc);
void clk_sr_pre(struct Calc* calc);
void clk_sr_post(struct Calc* calc);

void clk_sr_pre(struct Calc* calc)
{
    unsigned a, b, c;
    a = calc->dat[1];
    b = calc->dat[26];
    c = calc->dat[23];
    unsigned B2 = ( a & (~b) );
    unsigned A = (  b   & (~c) );
    A |= B2;

    calc->sig1 = A;
}

void clk_sr_post(struct Calc* calc)
{
    memcpyN(calc->dat+1, calc->dat, NUM_OUTPUTS-1);
    calc->dat[0] = calc->sig1;
}

unsigned calc_output(struct Calc* calc)
{
    return calc->dat[10];
}

void set_state(unsigned* data, struct Calc* calc)
{
    for(unsigned i = 0; i < NUM_OUTPUTS; i++) {
        calc->dat[i] = data[i];
    }
}

void fullclock (
    unsigned* state
) {
    struct Calc calc;
    set_state(state, &calc);

    unsigned atOutput = 0;
    for (int i = 0; i < NUM_OUTPUTS; i++) {
        clk_sr_pre(&calc);
        state[atOutput++] = calc_output(&calc);
        clk_sr_post(&calc);
    }
}
