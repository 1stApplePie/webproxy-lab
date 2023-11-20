/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *query_string = getenv("QUERY_STRING");

    if (query_string == NULL) {
        fprintf(stderr, "Error: No query string\n");
        return 1;
    }

    int a, b;
    // query_string에서 a와 b의 값을 읽어옴
    if (sscanf(query_string, "a=%d&b=%d", &a, &b) != 2) {
        fprintf(stderr, "Error: Invalid input\n");
        return 1;
    }

    printf("Content-Type: text/plain\n\n"); // 헤더 출력
    printf("Sum: %d\n", a + b);

    return 0;
}
/* $end adder */
