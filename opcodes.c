
void add(unsigned char amount) {
    regs[REG_A] += amount;

    set_z_flag();
    
    regs[REG_F] |= 0 << FLAG_N;
    printf("%d, %d", regs[REG_A], amount);
    if ((regs[REG_A]) < 0) {
	regs[REG_F] |= 1 << FLAG_C;
    } else {
	regs[REG_F] |= 0 << FLAG_C;
    }
}
