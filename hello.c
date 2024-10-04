#include <stdio.h>

int main()
{
    printf("Hello, World!\n");
    return 0;
    
}
int fibonacci(int n) {
    // Check for edge cases
    if (n <= 0) {
        return -1;  // Invalid input
    } else if (n == 1) {
        return 0;
    } else if (n == 2) {
        return 1;
    }

    // Initialize the first two Fibonacci numbers
    int a = 0, b = 1, next;

    // Calculate Fibonacci sequence up to n
    for (int i = 3; i <= n; i++) {
        next = a + b;
        a = b;
        b = next;
    }

    return b;
}


