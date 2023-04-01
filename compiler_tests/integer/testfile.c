int f()
{
    int x;
    x=5678;
    {
        int x;
        int y;
        x=1234;
    }
    return x;
}
