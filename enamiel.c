


// ENAMIEL OS - Version C v3.0 - UN SEUL FICHIER
// Toutes les fonctionnalites Python, mais en vrai C

__asm__(".globl _start\n_start:\n call main\n cli\n hlt\n");

volatile char* video = (volatile char*)0xB8000;
int cursor = 0;
unsigned char couleur_actuelle = 0x07;

unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void print(const char* str) {
    int i = 0;
    while(str[i] != 0) {
        if(str[i] == '\n') {
            cursor = (cursor / 160 + 1) * 160;
        } else {
            video[cursor] = str[i];
            video[cursor+1] = couleur_actuelle;
            cursor += 2;
        }
        i++;
        if(cursor >= 80*25*2) cursor = 0;
    }
}

void clear() {
    for(int i = 0; i < 80*25*2; i+=2) {
        video[i] = ' ';
        video[i+1] = couleur_actuelle;
    }
    cursor = 0;
}

int egal(const char* a, const char* b) {
    int i = 0;
    while(a[i] != 0 && b[i] != 0) {
        if(a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

void print_nombre(int n) {
    char buf[20]; int i = 0;
    if(n==0) { print("0"); return; }
    if(n<0) { print("-"); n=-n; }
    while(n>0) { buf[i++] = '0' + (n%10); n/=10; }
    for(int j=i-1;j>=0;j--) { char s[2]={buf[j],0}; print(s); }
}

char get_key() {
    char keys[] = "??1234567890-=?qwertyuiop[]\n?asdfghjkl;\'`?\\zxcvbnm,./?*? ?";
    char keys_shift[] = "??!@#$%^&*()_+?QWERTYUIOP{}\n?ASDFGHJKL:\"~?|ZXCVBNM<>??*? ?";
    int shift = 0;
    while(1) {
        if(inb(0x64) & 1) {
            unsigned char code = inb(0x60);
            if(code == 0x2A || code == 0x36) shift = 1;
            if(code == 0xAA || code == 0xB6) shift = 0;
            if(code < 58 && code > 0) {
                if(shift) return keys_shift[code];
                else return keys[code];
            }
        }
    }
}

int lire_cmos(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}
int bcd_vers_dec(int v) { return (v & 0x0F) + ((v >> 4) * 10); }
int aleatoire(int max) {
    int t = lire_cmos(0) + lire_cmos(2)*60 + lire_cmos(4)*3600;
    return (t * 1103515245 + 12345) % max + 1;
}

char notes[2000];
int notes_len = 0;
char buffer[100];
int buf_pos = 0;

void lire_ligne(char* dest, int max) {
    int pos = 0;
    while(1) {
        char c = get_key();
        if(c == '\n') { dest[pos]=0; print("\n"); return; }
        if(c != '?' && pos < max-1) {
            dest[pos]=c; pos++;
            char s[2]={c,0}; print(s);
        }
    }
}

void jeu() {
    int nombre = aleatoire(10);
    print("J'ai choisi un nombre entre 1 et 10. Devine !\n");
    while(1) {
        print("Ton nombre : ");
        char inp[10]; lire_ligne(inp, 10);
        int essai = 0;
        for(int i=0; inp[i]!=0; i++) essai = essai*10 + (inp[i]-'0');
        if(essai == nombre) { print("Bravo ! Tu as gagne !\n"); break; }
        else if(essai < nombre) print("C'est plus grand !\n");
        else print("C'est plus petit !\n");
    }
}

void calcul() {
    print("Tape ton calcul (ex: 5+5) : ");
    char inp[20]; lire_ligne(inp, 20);
    int a=0, b=0, i=0, op=0;
    while(inp[i]>='0' && inp[i]<='9') { a=a*10+(inp[i]-'0'); i++; }
    op = inp[i]; i++;
    while(inp[i]>='0' && inp[i]<='9') { b=b*10+(inp[i]-'0'); i++; }
    int res=0;
    if(op=='+') res=a+b;
    if(op=='-') res=a-b;
    if(op=='*') res=a*b;
    if(op=='/') res = b!=0 ? a/b : 0;
    print("Resultat : "); print_nombre(res); print("\n");
}

void check_command() {
    buffer[buf_pos]=0;
    print("\n");
    if(egal(buffer, "aide")) {
        print("Commandes : aide, version, bonjour, effacer, quitter, calcul, heure, couleur, note, lire, jeu\n");
    } else if(egal(buffer, "version")) {
        print("ENAMIEL OS v3.0 en C - Fait par toi\n");
    } else if(egal(buffer, "bonjour")) {
        print("Salut ! C'est ENAMIEL en C qui te parle\n");
    } else if(egal(buffer, "effacer")) {
        clear();
    } else if(egal(buffer, "heure")) {
        print("Il est ");
        print_nombre(bcd_vers_dec(lire_cmos(4)));
        print("h");
        print_nombre(bcd_vers_dec(lire_cmos(2)));
        print("\n");
    } else if(egal(buffer, "calcul")) {
        calcul();
    } else if(egal(buffer, "couleur")) {
        print("Choisis : rouge, vert, bleu, normal\nCouleur ? ");
        char c[20]; lire_ligne(c, 20);
        if(egal(c, "rouge")) couleur_actuelle = 0x0C;
        else if(egal(c, "vert")) couleur_actuelle = 0x0A;
        else if(egal(c, "bleu")) couleur_actuelle = 0x09;
        else if(egal(c, "normal")) couleur_actuelle = 0x07;
        else print("Couleur inconnue\n");
        clear();
    } else if(egal(buffer, "note")) {
        print("Ecris ta note : ");
        char n[100]; lire_ligne(n, 100);
        for(int i=0; n[i]!=0; i++) notes[notes_len++] = n[i];
        notes[notes_len++] = '\n'; notes[notes_len]=0;
        print("Note enregistree !\n");
    } else if(egal(buffer, "lire")) {
        if(notes_len==0) print("Pas encore de note. Tape 'note' d'abord.\n");
        else { print("--- Tes notes ---\n"); print(notes); }
    } else if(egal(buffer, "jeu")) {
        jeu();
    } else if(egal(buffer, "quitter")) {
        print("Au revoir, merci d'avoir utilise ENAMIEL OS\n");
        while(1) {}
    } else {
        print("Commande inconnue. Tape 'aide'\n");
    }
    buf_pos = 0;
    print("ENAMIEL> ");
}

void main() {
    clear();
    print("=================================\n");
    print("  ENAMIEL OS - Connexion\n");
    print("=================================\n");
    print("Mot de passe (indice : enamiel) : ");
    char mdp[20]; lire_ligne(mdp, 20);
    if(!egal(mdp, "enamiel")) {
        print("Mot de passe faux ! Acces refuse.\n");
        while(1) {}
    }
    clear();
    print("=================================\n");
    print("  Bienvenue sur ENAMIEL OS\n");
    print("  Version C 3.0\n");
    print("=================================\n");
    print("Tape 'aide' pour voir les commandes\n");
    print("ENAMIEL> ");
    while(1) {
        char c = get_key();
        if(c == '\n') { check_command(); }
        else if(c != '?' && buf_pos < 99) {
            buffer[buf_pos]=c; buf_pos++;
            char s[2]={c,0}; print(s);
        }
    }
}
