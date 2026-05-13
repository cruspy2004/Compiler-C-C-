/*
 * follow.c  —  Module 4: Compute FOLLOW sets.
 *
 * Algorithm (fixed-point):
 *   1. Add $ to FOLLOW(start symbol).
 *   2. For every production A -> alpha B beta:
 *        Add FIRST(beta) - {eps} to FOLLOW(B).
 *        If eps in FIRST(beta) (or beta is empty): add FOLLOW(A) to FOLLOW(B).
 *   Repeat until stable.
 *
 * FIRST sets are also computed here (same algorithm as first.c) so this
 * binary is self-contained.
 */

#include <stdio.h>
#include <string.h>
#include "grammar.h"

#define MAX_SET 40

/* ---- Set utilities ---- */
typedef struct { char sym; char m[MAX_SET]; int sz; } Set;
static Set first_s[MAX_SYMBOLS];
static Set follow_s[MAX_SYMBOLS];
static int nf;

static void sadd(Set *s, char c) {
    for (int i=0;i<s->sz;i++) if(s->m[i]==c) return;
    s->m[s->sz++]=c;
}
static int shas(Set *s, char c) {
    for(int i=0;i<s->sz;i++) if(s->m[i]==c) return 1; return 0;
}
static Set *fset(Set arr[], int n, char c) {
    for(int i=0;i<n;i++) if(arr[i].sym==c) return &arr[i]; return NULL;
}

/* ---- Compute FIRST (needed for FOLLOW computation) ---- */
static void compute_first(Grammar *g) {
    nf = g->num_nts;
    for(int i=0;i<g->num_nts;i++){first_s[i].sym=g->non_terminals[i]; first_s[i].sz=0;}
    int changed=1;
    while(changed){
        changed=0;
        for(int pi=0;pi<g->num_prods;pi++){
            Production *p=&g->prods[pi];
            Set *fs=fset(first_s,nf,p->lhs);
            for(int ri=0;ri<p->num_prods;ri++){
                const char *rhs=p->rhs[ri];
                if(rhs[0]==EPSILON){ if(!shas(fs,EPSILON)){sadd(fs,EPSILON);changed=1;} continue; }
                int all=1;
                for(int si=0;rhs[si];si++){
                    char s=rhs[si];
                    if(is_terminal(s)){ if(!shas(fs,s)){sadd(fs,s);changed=1;} all=0; break; }
                    Set *sub=fset(first_s,nf,s);
                    if(sub){ for(int k=0;k<sub->sz;k++) if(sub->m[k]!=EPSILON&&!shas(fs,sub->m[k])){sadd(fs,sub->m[k]);changed=1;} if(!shas(sub,EPSILON)){all=0;break;} }
                    else{all=0;break;}
                }
                if(all&&!shas(fs,EPSILON)){sadd(fs,EPSILON);changed=1;}
            }
        }
    }
}

/* Compute FIRST of a string (sequence of symbols starting at rhs+pos). */
static void first_of_string(Grammar *g, const char *rhs, int pos, Set *out) {
    if(!rhs[pos]){ sadd(out,EPSILON); return; }
    int all=1;
    for(int si=pos;rhs[si];si++){
        char s=rhs[si];
        if(s==EPSILON){ sadd(out,EPSILON); return; }
        if(is_terminal(s)){ sadd(out,s); all=0; break; }
        Set *sub=fset(first_s,nf,s);
        if(sub){ for(int k=0;k<sub->sz;k++) if(sub->m[k]!=EPSILON) sadd(out,sub->m[k]); if(!shas(sub,EPSILON)){all=0;break;} }
        else{all=0;break;}
    }
    if(all) sadd(out,EPSILON);
}

static void compute_follow(Grammar *g) {
    for(int i=0;i<g->num_nts;i++){follow_s[i].sym=g->non_terminals[i]; follow_s[i].sz=0;}
    /* Rule 1 */
    Set *sf=fset(follow_s,g->num_nts,g->start);
    if(sf) sadd(sf,ENDMARK);

    int changed=1;
    while(changed){
        changed=0;
        for(int pi=0;pi<g->num_prods;pi++){
            Production *p=&g->prods[pi];
            Set *fA=fset(follow_s,g->num_nts,p->lhs);
            for(int ri=0;ri<p->num_prods;ri++){
                const char *rhs=p->rhs[ri];
                for(int si=0;rhs[si];si++){
                    char sym=rhs[si];
                    if(!is_nonterminal(sym)) continue;
                    Set *fB=fset(follow_s,g->num_nts,sym);
                    if(!fB) continue;
                    /* Add FIRST(beta) - eps to FOLLOW(B) */
                    Set beta_first; beta_first.sz=0;
                    first_of_string(g,rhs,si+1,&beta_first);
                    for(int k=0;k<beta_first.sz;k++){
                        if(beta_first.m[k]!=EPSILON&&!shas(fB,beta_first.m[k])){sadd(fB,beta_first.m[k]);changed=1;}
                    }
                    /* If beta can derive eps, add FOLLOW(A) */
                    if(shas(&beta_first,EPSILON)&&fA){
                        for(int k=0;k<fA->sz;k++) if(!shas(fB,fA->m[k])){sadd(fB,fA->m[k]);changed=1;}
                    }
                }
            }
        }
    }
}

int main(void) {
    Grammar g;
    grammar_load_default(&g);
    compute_first(&g);
    compute_follow(&g);

    printf("=== FOLLOW Sets ===\n");
    for(int i=0;i<g.num_nts;i++){
        Set *fs=fset(follow_s,g.num_nts,g.non_terminals[i]);
        if(!fs) continue;
        char nt=fs->sym;
        if(nt=='P') printf("FOLLOW(E') = { ");
        else if(nt=='Q') printf("FOLLOW(T') = { ");
        else printf("FOLLOW(%c)  = { ", nt);
        for(int j=0;j<fs->sz;j++){
            char c=fs->m[j];
            if(c==ENDMARK) printf("$");
            else if(c==EPSILON) printf("epsilon");
            else printf("%c",c);
            if(j<fs->sz-1) printf(", ");
        }
        printf(" }\n");
    }
    return 0;
}
