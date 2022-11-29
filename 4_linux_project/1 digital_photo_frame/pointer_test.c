typedef struct{
    int a;
    int b;
}T_AB;

int main(void)
{
    char c = 'B';
    int a;
    T_AB tTest;
    int *p;
    int **pp;
    /*1. 对指针变量赋值为整型变量的地址*/
    p = &a;
    printf("p = 0x%x, a'addr = 0x%x\n", p, &a);
    /*2. 改变指针变量所指整型变量的值*/
    *p = 0x12345678;
    printf("a = 0x%x\n", a);
    /*3. 让指针变量指向字符变量*/
    p = &c;
    printf("p = 0x%x, c'addr = 0x%x\n", p, &c);
    /*4. 改变指针变量所指字符变量的值*/
    *p = 'A';
    printf("c = %c\n", c);
    /*5. 指针变量指向结构体变量*/
    p = &tTest;
    printf("p = 0x%x, tTest'addr = 0x%x\n", p, &tTest);
    /*6. 通过指向结构体指针变量改变结构体变量的值*/
    *p = &tTest;
    printf("tTest.a = 0x%x, tTest'addr = 0x%x\n", tTest.a, &tTest);
    /*7.二级指针变量赋值*/
    pp = &p;
    printf("pp = 0x%x, p'addr = 0x%x\n", pp, &p);
    /*8.更改二级指针所指变量的值*/
    **pp = 0xABCD1234;
    printf("tTest.a = 0x%x\n", tTest.a);

    return 0;
}


