#include<stdio.h>
#include<assert.h>
#include<stdlib.h>

int taille(const char* s);

    char *palindrome(const char *s){
        char *ret = malloc((sizeof(char) * taille(s) * 2) + 1);
        assert(ret != NULL);
        int i = 0, j = 0;
        while(*s != '\0'){
            if(*s != ' '){
                ret[i] = *s;
                i++;
                j++;
            }
            s++;
        }
        s--;
        while(j > 0){
            if(*s != ' '){
                ret[i] = *s;
                i++;
                j--;
            }
            s--;
        }
        ret[i] = '\0';
        return ret;
    }

    int taille(const char *s){
        int compteur = 0;
        while(*s != '\0'){
            if(*s != ' ')
                compteur++;
            s++;
        }
        return compteur;
    }


    int main(void){
        char *s1 = palindrome(" aabca ");
        assert(s1 != NULL);
        printf("%s\n",s1);
        free(s1);
        return 0;
    }
