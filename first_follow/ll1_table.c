/*
 * ll1_table.c  —  Module 4: Build and print the LL(1) parsing table.
 *
 * For each production A -> alpha:
 *   For each terminal a in FIRST(alpha):
 *     Table[A][a] = "A -> alpha"
 *   If epsilon in FIRST(alpha):
 *     For each b in FOLLOW(A):
 *       Table[A][b] = "A -> alpha"
 *
 * Conflicts (two productions in same cell) are reported as errors.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "grammar.h"

#define MAX_SET 40
#define NO_PROD (-1)

typedef struct { char sym; char m[MAX_SET]; int sz; } Set;
static Set first_s[MAX_SYMBOLS];
static Set follow_s[MAX_SYMBOLS];
static int nf_g;

static void sadd(Set *s,char c){for(int i=0;i<s->sz;i++)if(s->m[i]==c)return;s->m[s->sz++]=c;}
static int  shas(Set *s,char c){for(int i=0;i<s->sz;i++)if(s->m[i]==c)return 1;return 0;}
static Set *fset(Set a[],int n,char c){for(int i=0;i<n;i++)if(a[i].sym==c)return &a[i];return NULL;}

static void compute_first(Grammar *g){
    nf_g=g->num_nts;
    for(int i=0;i<g->num_nts;i++){first_s[i].sym=g->non_terminals[i];first_s[i].sz=0;}
    int ch=1;
    while(ch){ch=0;for(int pi=0;pi<g->num_prods;pi++){Production *p=&g->prods[pi];Set *fs=fset(first_s,nf_g,p->lhs);
    for(int ri=0;ri<p->num_prods;ri++){const char *rhs=p->rhs[ri];if(rhs[0]==EPSILON){if(!shas(fs,EPSILON)){sadd(fs,EPSILON);ch=1;}continue;}
    int all=1;for(int si=0;rhs[si];si++){char s=rhs[si];if(is_terminal(s)){if(!shas(fs,s)){sadd(fs,s);ch=1;}all=0;break;}
    Set *sub=fset(first_s,nf_g,s);if(sub){for(int k=0;k<sub->sz;k++)if(sub->m[k]!=EPSILON&&!shas(fs,sub->m[k])){sadd(fs,sub->m[k]);ch=1;}if(!shas(sub,EPSILON)){all=0;break;}}else{all=0;break;}}
    if(all&&!shas(fs,EPSILON)){sadd(fs,EPSILON);ch=1;}}}}
}

static void first_of_string(Grammar *g,const char *rhs,int pos,Set *out){
    (void)g;
    int all=1;for(int si=pos;rhs[si];si++){char s=rhs[si];if(s==EPSILON){sadd(out,EPSILON);return;}
    if(is_terminal(s)){sadd(out,s);all=0;break;}Set *sub=fset(first_s,nf_g,s);
    if(sub){for(int k=0;k<sub->sz;k++)if(sub->m[k]!=EPSILON)sadd(out,sub->m[k]);if(!shas(sub,EPSILON)){all=0;break;}}else{all=0;break;}}
    if(all)sadd(out,EPSILON);
}

static void compute_follow(Grammar *g){
    for(int i=0;i<g->num_nts;i++){follow_s[i].sym=g->non_terminals[i];follow_s[i].sz=0;}
    Set *sf=fset(follow_s,g->num_nts,g->start);if(sf)sadd(sf,ENDMARK);
    int ch=1;while(ch){ch=0;for(int pi=0;pi<g->num_prods;pi++){Production *p=&g->prods[pi];Set *fA=fset(follow_s,g->num_nts,p->lhs);
    for(int ri=0;ri<p->num_prods;ri++){const char *rhs=p->rhs[ri];for(int si=0;rhs[si];si++){char sym=rhs[si];if(!is_nonterminal(sym))continue;
    Set *fB=fset(follow_s,g->num_nts,sym);if(!fB)continue;Set bf;bf.sz=0;first_of_string(g,rhs,si+1,&bf);
    for(int k=0;k<bf.sz;k++)if(bf.m[k]!=EPSILON&&!shas(fB,bf.m[k])){sadd(fB,bf.m[k]);ch=1;}
    if(shas(&bf,EPSILON)&&fA)for(int k=0;k<fA->sz;k++)if(!shas(fB,fA->m[k])){sadd(fB,fA->m[k]);ch=1;}}}}
    }
}

/* Table: table[nt_idx][term_idx] = production index (into prods array) or NO_PROD */
static int table[MAX_SYMBOLS][MAX_TERMINALS];

static void build_table(Grammar *g) {
    /* Initialise */
    for(int i=0;i<g->num_nts;i++)
        for(int j=0;j<g->num_terms;j++)
            table[i][j]=NO_PROD;

    for(int pi=0;pi<g->num_prods;pi++){
        Production *p=&g->prods[pi];
        /* find NT index */
        int nt_idx=-1;
        for(int i=0;i<g->num_nts;i++) if(g->non_terminals[i]==p->lhs){nt_idx=i;break;}
        if(nt_idx<0) continue;

        for(int ri=0;ri<p->num_prods;ri++){
            const char *rhs=p->rhs[ri];
            Set alpha_first; alpha_first.sz=0;
            first_of_string(g,rhs,0,&alpha_first);

            for(int k=0;k<alpha_first.sz;k++){
                char a=alpha_first.m[k];
                if(a==EPSILON) continue;
                int ti=-1;
                for(int j=0;j<g->num_terms;j++) if(g->terminals[j]==a){ti=j;break;}
                if(ti<0) continue;
                if(table[nt_idx][ti]!=NO_PROD)
                    fprintf(stderr,"LL(1) CONFLICT at [%c][%c]\n",p->lhs,a);
                else
                    table[nt_idx][ti]= (pi << 8) | ri;
            }
            if(shas(&alpha_first,EPSILON)){
                Set *fw=fset(follow_s,g->num_nts,p->lhs);
                if(!fw) continue;
                for(int k=0;k<fw->sz;k++){
                    char b=fw->m[k];
                    int ti=-1;
                    for(int j=0;j<g->num_terms;j++) if(g->terminals[j]==b){ti=j;break;}
                    if(ti<0) continue;
                    if(table[nt_idx][ti]!=NO_PROD)
                        fprintf(stderr,"LL(1) CONFLICT at [%c][%c]\n",p->lhs,b);
                    else
                        table[nt_idx][ti]=(pi<<8)|ri;
                }
            }
        }
    }
}

static void print_table(Grammar *g) {
    /* Header */
    printf("\n=== LL(1) Parsing Table ===\n\n");
    printf("%-6s", "NT");
    for(int j=0;j<g->num_terms;j++){
        char c=g->terminals[j];
        if(c==ENDMARK) printf("%-14s","$");
        else printf("%-14c",c);
    }
    printf("\n");
    for(int i=0;i<g->num_nts+g->num_terms;i++) printf("--");
    printf("\n");

    for(int i=0;i<g->num_nts;i++){
        char nt=g->non_terminals[i];
        if(nt=='P') printf("%-6s","E'");
        else if(nt=='Q') printf("%-6s","T'");
        else printf("%-6c",nt);

        for(int j=0;j<g->num_terms;j++){
            int entry=table[i][j];
            if(entry==NO_PROD){ printf("%-14s",""); continue; }
            int pi=entry>>8, ri=entry&0xFF;
            Production *p=&g->prods[pi];
            char buf[32];
            char lhs_str[4];
            if(p->lhs=='P') strcpy(lhs_str,"E'");
            else if(p->lhs=='Q') strcpy(lhs_str,"T'");
            else {lhs_str[0]=p->lhs;lhs_str[1]='\0';}
            /* Replace P/Q in RHS display */
            char rhs_disp[MAX_RHS_LEN+2];
            const char *rhs=p->rhs[ri];
            int di=0;
            for(int ci=0;rhs[ci];ci++){
                if(rhs[ci]=='P'){rhs_disp[di++]='E';rhs_disp[di++]='\'';}
                else if(rhs[ci]=='Q'){rhs_disp[di++]='T';rhs_disp[di++]='\'';}
                else if(rhs[ci]=='e'){rhs_disp[di++]='e';rhs_disp[di++]='p';rhs_disp[di++]='s';}
                else if(rhs[ci]=='i'){rhs_disp[di++]='i';rhs_disp[di++]='d';}
                else if(rhs[ci]=='n'){rhs_disp[di++]='n';rhs_disp[di++]='u';rhs_disp[di++]='m';}
                else rhs_disp[di++]=rhs[ci];
            }
            rhs_disp[di]='\0';
            snprintf(buf,sizeof(buf),"%s->%s",lhs_str,rhs_disp);
            printf("%-14s",buf);
        }
        printf("\n");
    }
}

int main(void) {
    Grammar g;
    grammar_load_default(&g);
    compute_first(&g);
    compute_follow(&g);
    build_table(&g);
    print_table(&g);
    return 0;
}
