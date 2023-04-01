int f(int n)
{
    if(n==0){
        return 0;
    }
    return f(n-1)+n;
}
