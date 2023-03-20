#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>

using namespace std;

const int m = 6;
const int BR_KLJUCEVA = m - 1;      //za umetanje
const int MIN_LIST = m / 2;         //za brisanje
const int MIN_UNUT = m / 2 - 1;
const int MIN_KOREN = 1;

class Zapis {
public:
    int b_id, c_id, tax_st;
    string name;
    string bal;

    Zapis(int b, int c, int tax, string ime, string balans) {
        b_id = b;
        c_id = c;
        tax_st = tax;
        name = ime;
        bal = balans;
    }

    void ispis(ostream &os = cout) const {
        os << 4300000000 + b_id << "|" << 4300000000 + c_id << "|" << name << "|" << tax_st << "|" << bal << endl;
    }
};

class Cvor {
public:
    int *keys;
    Cvor **deca;        //pokazivaci na decu
    Zapis **podaci;     //pokazivaci na zapise
    Cvor *otac;
    int n;              //broj kljuceva koji su trenutno u cvoru
    bool list = false;

    Cvor() {
        keys = new int[m - 1];
        deca = new Cvor *[m];
        podaci = new Zapis *[m];
        for (int i = 0; i < m; i++) {
            deca[i] = nullptr;
            podaci[i] = nullptr;
        }
        otac = nullptr;
        n = 0;
    }

    ~Cvor() {
        cout << "Brisanje: " << keys[0] << endl;
        delete[] keys;
        for (int i = 0; i < n + 1; i++) delete deca[i];
        delete[] podaci;
        podaci = nullptr;
        deca = nullptr;
        otac = nullptr;
    }
};

//red
void INSERT(Cvor *Q[], int *len, Cvor *n) {
    Q[*len] = n;
    (*len)++;
}

Cvor *DELETE(Cvor *Q[], int *len) {
    Cvor *x = Q[0];
    for (int i = 0; i < (*len) - 1; i++) Q[i] = Q[i + 1];
    (*len)--;
    return x;
}

void LEVEL_ORDER(Cvor *root) {
    if (!root) {
        cout << "Stablo je prazno!" << endl;
        return;
    }
    cout << "\nB+ stablo:\n";
    Cvor *Q[100000], *next;
    int n = 0, i;
    INSERT(Q, &n, root);
    INSERT(Q, &n, nullptr);              //granicnik
    while (n) {                           //dok nije prazan red
        next = DELETE(Q, &n);
        if (next == nullptr) {
            if (n > 1) {                   //ako ima jednog cvora znaci da je ostao samo granicnik
                INSERT(Q, &n,
                       nullptr);   //u trenutku kad nailazimo na granicnik dodajemo novi, jer to znaci da je kraj tog nivoa
                printf("\n");
            } else {
                cout << endl << endl;
                return;              //to znaci da se doslo do poslednjeg granicnika, i da nema vise cvorova u stablu
            }
        } else {
            cout << "[";
            for (i = 0; i < next->n; i++) {
                cout << next->keys[i];
                if (i != next->n - 1) cout << "|";
                if (!next->list) INSERT(Q, &n, next->deca[i]);
            }
            cout << "]";
            if (!next->list) {
                INSERT(Q, &n, next->deca[i]);
                cout << " ";
            } else if (Q[0])cout << "->";
        }
    }
}

///UMETANJE
void na_mesto_u_cvoru(Cvor *next, int key, Zapis *novi = nullptr) {
    next->n++;
    int i = next->n - 2;
    while (i >= 0 && next->keys[i] > key) {
        if (i == next->n - 2) {
            if (novi) next->podaci[i + 2] = next->podaci[i + 1];
            else next->deca[i + 2] = next->deca[i + 1];
        }
        next->keys[i + 1] = next->keys[i];
        if (novi) next->podaci[i + 1] = next->podaci[i];
        else next->deca[i + 1] = next->deca[i];
        i--;
    }
    next->keys[i + 1] = key;
    if (novi)
        next->podaci[i +
                     1] = novi;                   //uslov je jer moze da se ubacuje u cvor grananja, a u tom slucaju je novi=nullptr
    else next->deca[i + 1] = next->deca[i];;
}

/*
ZA SPLIT LISTA:
    * alocira se novi cvor (njegov otac je isti kao otac prvog cvora)
    * pravi se pomocna lista svih kljuceva, da bi se novi kljuc koji se ubacuje ubacio sa ostalima
    * u prvi ide mid kljuceva, a u drugi ostatak
    * u jedan od ta dva cvora se ubacuje novi kljuc, tj. tu mora da se ubaci novi zapis, dok se veci kljucevi pomeraju na desno
    * u roditeljski cvor treba da se prebaci max iz prvog(mid), a pri tome moze da dodje do splita na svim nivoima do korena
    * treba da se uradi i azuriranje pokazivaca na decu u roditeljskom cvoru (levo i desno od repliciranog mid kljuca)
    * treba da se azuriraju pokazivaci na sledeci clan ulancane liste (drugi pokazuje na onog na koji je prvi, a prvi na drugi)
 */

int split_cvor_grananja(Cvor *root, Cvor *prvi, Cvor *drugi, int key, Cvor *dete1, Cvor *dete2) {
    int *pomocna = new int[m];                        //BR_KLJUCEVA + 1 za onaj koji se ubacuje = m
    int br = 0, br2 = 0;
    Cvor **pom_deca = new Cvor *[m];
    bool ubacen = false;
    for (int i = 0; i < prvi->n; i++) {
        if (key > prvi->keys[i]) {
            pom_deca[br2++] = prvi->deca[i];
            pomocna[br++] = prvi->keys[i];
        } else {
            if (!ubacen) {                           //dodavanje novog kljuca i deteta u pomocne liste
                pom_deca[br2++] = dete1;
                pom_deca[br2++] = dete2;
                pomocna[br++] = key;
                pomocna[br++] = prvi->keys[i];
                ubacen = true;
            } else {
                pom_deca[br2++] = prvi->deca[i];
                pomocna[br++] = prvi->keys[i];
            }
        }
    }
    if (key > prvi->keys[prvi->n - 1]) {
        pom_deca[br2++] = dete1;
        pom_deca[br2++] = dete2;
        pomocna[br++] = key;
    } else {
        pom_deca[br2++] = prvi->deca[prvi->n];
    }
    cout << dete2->keys[0] << endl;
    for (int i = 0; i < br; i++) cout << pomocna[i] << " ";
    cout << endl;
    for (int i = 0; i < br2; i++) cout << pom_deca[i]->keys[0] << " ";
    cout << endl;
    int mid, i;
    if (m % 2 == 0) mid = m / 2 - 1;
    else mid = m / 2;
    prvi->n = mid;
    drugi->n = m - 1 - prvi->n;
    for (i = 0; i < prvi->n; i++) {
        prvi->keys[i] = pomocna[i];
        prvi->deca[i] = pom_deca[i];
        pom_deca[i]->otac = prvi;
    }
    prvi->deca[mid] = pom_deca[i];
    pom_deca[i]->otac = prvi;
    br = 0;
    //cout << pom_deca[5]->keys[0] << endl;
    for (i = mid + 1; i < m + 1; i++) {
        drugi->keys[br] = pomocna[i];                           //ubacivanje kljuceva u drugi
        drugi->deca[br++] = pom_deca[i];
        if (pom_deca[i]->otac)pom_deca[i]->otac = drugi;
        //cout << br << endl;
    }
    drugi->otac = prvi->otac;
    prvi->list = false;
    drugi->list = false;
    return pomocna[mid];        //kljuc koji se prebacuje u oca
}

void split_list(Cvor *root, Cvor *prvi, Cvor *drugi, Zapis *novi, int key) {
    int *pomocna = new int[m];                //BR_KLJUCEVA + 1 za onaj koji se ubacuje = m
    int u_prvom = 0, br = 0;
    bool ubacen = false;
    for (int i = 0; i < prvi->n; i++) {
        if (key > prvi->keys[i]) pomocna[br++] = prvi->keys[i];
        else {
            if (!ubacen) {
                pomocna[br++] = key;
                ubacen = true;
            }             //dodavanje novog kljuca u listu kljuceva
            pomocna[br++] = prvi->keys[i];
        }
    }
    if (key > prvi->keys[prvi->n - 1]) pomocna[br] = key;
    int mid;
    if (m % 2 == 0) mid = m / 2 - 1;
    else mid = m / 2;
    Cvor *temp = prvi->deca[m - 1];                   //cvor na koji sada treba da pokazuje cvor drugi u ul. listi
    prvi->n = mid + 1;
    drugi->n = m - prvi->n;

    br = 0;
    for (int i = mid + 1; i < m; i++)
        drugi->keys[br++] = pomocna[i];                         //ubacivanje kljuceva u drugi
    br = 0;
    int i;
    for (i = 0; i < drugi->n; i++) {                                                   //ubacivanje zapisa u drugi
        if (drugi->keys[br] == key) drugi->podaci[br] = novi;
        else drugi->podaci[br] = prvi->podaci[mid + 1 + i];
        br++;
    }
    drugi->podaci[br] = prvi->podaci[mid + 1 +
                                     i];                                   //ubacivanje poslednjeg podatka (za 1 vise od kljuceva)

    if (pomocna[mid] >= key) u_prvom = 1;
    if (u_prvom) {
        na_mesto_u_cvoru(prvi, key,
                         novi);        //ukoliko nije ubacen u novi, znaci da treba da se ubaci u stari (prvi)
        prvi->n--;          //smanjujem jer sam ga u f-ji na_mesto_u_cvoru vec povecala a nije potrebno
    }
    prvi->deca[m - 1] = drugi;
    drugi->deca[m - 1] = temp;
    drugi->otac = prvi->otac;                             //dodavanje oca
    prvi->list = true;
    drugi->list = true;
}

Cvor *umetanje_u_cvor_grananja(Cvor *root, Cvor *next, int key, Cvor *dete1, Cvor *dete2) {
    while (next && next->n == m - 1) {
        Cvor *drugi = new Cvor(), *prvi = next;                      //alociranje prostora za drugi cvor
        key = split_cvor_grananja(root, prvi, drugi, key, dete1, dete2);
        drugi->list = false;
        dete1 = prvi;
        dete2 = drugi;
        next = next->otac;
    }
    if (next == nullptr) {                  //dosli smo do korena
        Cvor *novi = new Cvor;
        novi->keys[0] = key;
        novi->n++;
        novi->deca[0] = dete1;
        dete1->otac = novi;
        novi->deca[1] = dete2;
        dete2->otac = novi;
        cout << " U koren" << endl;
        root = novi;                      //jedini ko nema oca je koren, pa znamo da ce novonastali cvor biti novi koren
        cout << root->keys[0] << endl;
        novi->list = false;
        return root;
    } else {                            //cvor nije pun (samo se dodaje)
        na_mesto_u_cvoru(next, key);
        int i;
        for (i = 0; i < next->n; i++) if (next->keys[i] == key) break;
        next->deca[i] = dete1;
        dete1->otac = next;              //bitno je da se azuriraju pokazivaci na oca
        next->deca[i + 1] = dete2;
        dete2->otac = next;
    }
    return root;
}

Cvor *umetanje_u_list(Cvor *root, Cvor *next, Zapis *novi, int key) {
    if (next->n != m - 1) na_mesto_u_cvoru(next, key, novi);           //list nije pun (samo se dodaje)
    else {
        Cvor *drugi = new Cvor(), *prvi = next;                      //alociranje prostora za drugi cvor
        split_list(root, prvi, drugi, novi, key);
        int kopija = prvi->keys[prvi->n - 1];                       //taj cvor se kopira u cvor oca
        drugi->list = true;
        drugi->otac = prvi->otac;
        root = umetanje_u_cvor_grananja(root, prvi->otac, kopija, prvi, drugi);
    }
    return root;
}

Cvor *umetanje_u_stablo(Cvor *root, Zapis *novi, int key) {
    if (root == nullptr) {                                  //slucaj 1: prazno stablo
        root = new Cvor;
        root->keys[0] = key;
        root->podaci[0] = novi;
        root->n++;
        root->list = true;
        return root;
    }
    Cvor *next = root;
    while (!next->list) {                                 //slucaj 2: cvor grananja -> prvo se pronalazi list
        int i = 0;
        while (i < next->n && next->keys[i] < key) i++;
        next = next->deca[i];
    }
    root = umetanje_u_list(root, next, novi, key);      //slucaj 3: list -> umece se uvek u list
    cout << root->keys[0] << endl;
    return root;
}


///BRISANJE
void zamena_u_pretku(Cvor *next, bool flag = false) {
    int i, found = false, x = 0;
    Cvor *otac = next->otac;
    if (flag) x = 2;                           //slucaj kada se ubacuje iz desnog cvora (jer je tu kljuc ubacen, a ne izbacen pa su drugaciji indeksi
    while (!found) {                         //trazi se predak u kom se nalazi taj kljuc (ne mora da bude direktan otac)
        if (otac) {
            for (i = 0; i < otac->n; i++)
                if (otac->keys[i] == next->keys[next->n - x]) {      //trazenje tog kljuca u ocu, pa zamena
                    found = true;
                    if (next->n > 1) otac->keys[i] = next->keys[next->n - 1];
                }
            if (!found) otac = otac->otac;         //ako nije na tom nivou trazi se na sledecem
        } else return;
    }
}

Cvor *spajanje_cvorova_grananja(Cvor *root, Cvor *levi, Cvor *desni, int ind) {
    cout << "SPAJANJE\n";
    Cvor *otac = levi->otac;
    int i;
    int br = levi->n;
    levi->keys[br++] = otac->keys[ind];

    for (i = 0; i < desni->n; i++) {                           //prebacivanje svega iz desnog u levog brata
        levi->keys[br] = desni->keys[i];
        levi->deca[br++] = desni->deca[i];
        desni->deca[i]->otac = levi;
    }
    levi->deca[br++] = desni->deca[i];
    desni->deca[i]->otac = levi;
    levi->n = br - 1;

    for (i = ind; i < otac->n - 1; i++) otac->keys[i] = otac->keys[i + 1];
    for (int j = ind + 1; j < otac->n; j++) otac->deca[j] = otac->deca[j + 1];
    //otac->deca[i]=otac->deca[i+1];
    otac->n--;

    for (i = 0; i < desni->n + 1; i++) desni->deca[i] = nullptr;
    delete desni;                                       //brise se drugi, jer se od 2 cvora dobija 1
    return root;
}

Cvor *spajanje_listova(Cvor *root, Cvor *levi, Cvor *desni, int ind) {
    Cvor *otac = levi->otac;
    int i;
    int br = levi->n;
    for (i = 0; i < desni->n; i++) {                           //prebacivanje svega iz desnog u levog brata
        levi->keys[br] = desni->keys[i];
        levi->podaci[br++] = desni->podaci[i];
    }
    levi->podaci[br++] = desni->podaci[i];
    levi->n = br - 1;
    levi->deca[m - 1] = desni->deca[m - 1];      //pokazivac na sledeceg clana ulancane liste
    otac->deca[ind] = levi;                               //u ocu brisemo jedan razdvojni kljuc, pa pomeramo sve posle njega
    levi->otac = otac;
    for (i = ind; i < otac->n - 1; i++) {
        otac->keys[i] = otac->keys[i + 1];
        otac->deca[i] = otac->deca[i + 1];
    }
    otac->deca[i] = otac->deca[i + 1];
    otac->n--;

    for (i = 0; i < desni->n + 1; i++) desni->deca[i] = nullptr;
    delete desni;                                                   //brise se drugi, jer se od 2 cvora dobija 1
    return root;
};

Cvor *brisanje_unutrasnji(Cvor *root, Cvor *next, int key) {
    Cvor *desni_brat = nullptr, *levi_brat = nullptr, *otac;
    int i, j;
    //otac->n++;
    while (next->otac && next->n < MIN_UNUT) {
        otac = next->otac;
        for (i = 0; i < otac->n; i++) if (otac->keys[i] >= key) break;        //levo odatle je sin (na istom indeksu i)
        int levi = i - 1, desni = i + 1;
        if (desni <= otac->n) desni_brat = otac->deca[desni];
        else if (levi >= 0) levi_brat = otac->deca[levi];

        ///1. slucaj - uzimanje iz desnog brata
        if (desni_brat && desni_brat->n >= MIN_UNUT + 1) {
            cout << "Prebacivanje iz desnog cvora grananja\n";
            int temp = desni_brat->keys[0];                                        //prebacivanje iz brata prvog kljuca
            Cvor *temp_dete = desni_brat->deca[0];                                 //prebacuje se i prvo dete

            //brisanje prvog kjuca - shiftovanje
            for (j = 0; j < desni_brat->n - 1; j++) {
                desni_brat->keys[j] = desni_brat->keys[j + 1];
                desni_brat->deca[j] = desni_brat->deca[j + 1];
            }
            desni_brat->deca[j] = desni_brat->deca[j +
                                                   1];                           //uvek kod prebacivanja dece, jer dece ima za 1 vise od kljuceva
            desni_brat->n--;

            //kaskadno prebacivanje
            next->n++;
            next->keys[next->n - 1] = otac->keys[i];
            next->deca[next->n] = temp_dete;
            temp_dete->otac = next;
            otac->keys[i] = temp;
        }

            ///2. slucaj - uzimanje iz levog brata
        else if (levi_brat && levi_brat->n >= MIN_LIST + 1) {
            cout << "Prebacivanje iz levog cvora grananja\n";
            int temp = levi_brat->keys[levi_brat->n -
                                       1];                                      //prebacivanje iz brata prvog kljuca
            Cvor *temp_dete = levi_brat->deca[levi_brat->n];                                 //prebacuje se i prvo dete

            //umetanje u next - shiftovanje
            for (j = next->n; j >= 1; j--) {
                next->keys[j] = next->keys[j - 1];
                next->deca[j] = next->deca[j - 1];
            }
            next->deca[j] = next->deca[j - 1];

            //kaskadno prebacivanje
            levi_brat->n--;
            next->n++;
            next->keys[0] = otac->keys[i];
            next->deca[0] = temp_dete;
            temp_dete->otac = next;
            otac->keys[i] = temp;
            next->n++;
        } else {
            if (desni_brat) levi_brat = next;
            else {
                desni_brat = next;
                i = i - 1;
            }            //i treba da pokazuje na levog brata
            root = spajanje_cvorova_grananja(root, levi_brat, desni_brat, i);
        }
        next = otac;
        levi_brat = nullptr;
        desni_brat = nullptr;
    }
    if (!next->otac && next->n == 0) {             //to je koren, i prazan je
        root = next->deca[0];
    }
    return root;
}

Cvor *brisanje(Cvor *root, int key) {
    if (root == nullptr) {
        cout << "Stablo je prazno!\n";
        return root;
    }
    Cvor *next = root;
    int i;
    bool found = false;

    //pretraga
    while (!next->list) {                                 //brise se uvek iz lista
        i = 0;
        while (i < next->n && next->keys[i] < key) i++;
        next = next->deca[i];
    }
    for (i = 0; i < next->n; i++) {
        if (next->keys[i] == key) {
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Nema tog kljuca u stablu!\n";
        return root;
    }

    //brisanje tog kljuca iz niza - shiftovanje
    for (int j = i; j < next->n - 1; j++) {
        next->podaci[j] = next->podaci[j + 1];
        next->keys[j] = next->keys[j + 1];
    }

    //azuriranje maksimuma u pretku (indeksnom delu)
    Cvor *otac = next->otac;
    next->n--;
    if (next->keys[next->n] == key)
        zamena_u_pretku(next);       //da li je obrisan kljuc bio najveci u cvoru => zamena u cvoru pretka

    //provera dozvoljenog broja kljuceva
    if (next->n < MIN_LIST) {
        //prvo provera da li moze da se pozajmi od brace
        if (otac) {
            //pretraga u ocu
            for (i = 0; i < otac->n; i++) if (otac->keys[i] == next->keys[next->n - 1]) break;
            //ako nema tog kljuca u ocu (u nekom pretku je), onda je to svakako najdesniji sin, pa je i doslo do kraja (otac->n)

            int levi = i - 1, desni = i +
                                      1, j;                                                               //i je pozicija tog kljuca u ocu
            Cvor *desni_brat = nullptr, *levi_brat = nullptr;
            if (desni <= otac->n) desni_brat = otac->deca[desni];
            if (levi >= 0) levi_brat = otac->deca[levi];
            ///1. slucaj - uzimanje iz desnog brata
            if (desni_brat && desni_brat->n >= MIN_LIST + 1) {
                cout << "desni" << endl;
                next->keys[next->n] = desni_brat->keys[0];                                        //prebacivanje iz brata
                next->podaci[next->n] = desni_brat->podaci[0];
                for (j = 0; j < desni_brat->n - 1; j++) {
                    desni_brat->keys[j] = desni_brat->keys[j +
                                                           1];              //brisanje prvog kljuca iz desnog brata, jer smo ga prebacili u next
                    desni_brat->podaci[j] = desni_brat->podaci[j + 1];          //brisanje zapisa
                }
                desni_brat->podaci[j] = desni_brat->podaci[j + 1];
                desni_brat->n--;
                next->n++;
                //cout << "broj " << next->n << endl;
                zamena_u_pretku(next,
                                true);                                 //prebaceni kljuc mora da ide u pretka, jer je sad najveci u cvoru
            }

                ///2. slucaj - uzimanje iz levog brata
            else if (levi_brat && levi_brat->n >= MIN_LIST + 1) {
                cout << "levi" << endl;
                for (j = next->n; j >= 1; j--) {                                   //pomeranje svih kljuceva za 1 mesto
                    next->keys[j] = next->keys[j - 1];
                    next->podaci[j] = next->podaci[j - 1];
                }
                next->podaci[j] = next->podaci[j - 1];
                levi_brat->keys[levi_brat->n - 2];
                next->keys[0] = levi_brat->keys[levi_brat->n -
                                                1];          //prebacivanje iz brata na prvo mesto u cvoru next
                next->podaci[0] = levi_brat->podaci[levi_brat->n - 1];
                next->n++;
                levi_brat->n--;                                         //brisemo kljuc iz levog brata samo tako sto spustimo n
                zamena_u_pretku(levi_brat);
            }

                ///3. slucaj - spajanje
            else {                        //nije moguca pozajmica
                if (desni_brat) levi_brat = next;                              //ako postoji desni brat, spaja se sa njim, a ako ne onda se spaja sa levim
                else if (levi_brat) { desni_brat = next, i = i - 1; }

                spajanje_listova(root, levi_brat, desni_brat,
                                 i);              //i je indeks levog brata -> onaj u kog se vrsi spajanje
                cout << endl << "Nakon spajanja: " << endl;
                LEVEL_ORDER(root);
                otac->deca[i] = levi_brat;                                    //azuriranje u ocu
                if (!otac->otac) {
                    cout << "root\n";
                    if (otac->n == 0) {
                        root = levi_brat;
                        root->otac = nullptr;
                    }
                    return root;
                }
                root = brisanje_unutrasnji(root, otac, levi_brat->keys[levi_brat->n - 1]);
                LEVEL_ORDER(root);
                return root;
            }
        } else {
            //samo root nema oca
            cout << "Nema oca\n";
            if (next->n == 0) {
                cout << "Obrisano je celo stablo\n";
                delete root;
                return nullptr;
            }
        }
    }
    return root;
}

///PRETRAZIVANJE
int pretraga(Cvor *root, int key, ostream &os = cout) {
    if (root == nullptr) {
        cout << "Stablo je prazno!\n";
        return 0;
    }
    Cvor *next = root;
    int i, brojac_koraka = 0;
    while (!next->list) {
        i = 0;
        while (i < next->n && next->keys[i] < key) i++;
        next = next->deca[i];
        brojac_koraka++;
    }
    bool found = false;
    for (i = 0; i < next->n; i++) {
        brojac_koraka++;
        if (next->keys[i] == key) {
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Nema tog kljuca u stablu!\n";
        cout << "Broj koraka: " << brojac_koraka << endl;
        return brojac_koraka;
    }
    Zapis *podaci = next->podaci[i];
    os << 43000000000 + key << "|";
    podaci->ispis(os);
    cout << "Kljuc " << key << " je pronadjen u " << brojac_koraka << " koraka." << endl << endl;
    return brojac_koraka;
}

void visestruka_pretraga(Cvor *root) {
    int kljucevi[200], k, ukupan_broj_koraka = 0;
    cout << "Unesite broj kljuceva koji se pretrazuju: ";
    cin >> k;
    cout << "Unesite kljuceve: ";
    for (int i = 0; i < k; i++) cin >> kljucevi[i];
    ofstream f_out("izlazna.txt");
    for (int i = 0; i < k; i++) ukupan_broj_koraka += pretraga(root, kljucevi[i], f_out);
    cout << "Pretraga je zavrsena u ukupno " << ukupan_broj_koraka << " koraka." << endl << endl;
}

int sukcesivna_pretraga(Cvor *root) {
    if (root == nullptr) {
        cout << "Stablo je prazno!\n";
        return 0;
    }
    int kljucevi[200], k, ukupan_broj_koraka = 0, br = 0;
    cout << "Unesite broj kljuceva koji se pretrazuju: ";
    cin >> k;
    cout << "Unesite kljuceve u neopadajucem poretku: ";
    for (int i = 0; i < k; i++) cin >> kljucevi[i];
    ofstream f_out("izlazna.txt");

    //trazenje prvog, jer se odatle samo sukcesivno pretrazuje
    bool found = false;
    Cvor *next = root;
    int i;
    while (!found) {
        while (!next->list) {
            i = 0;
            while (i < next->n && next->keys[i] < kljucevi[br]) i++;
            next = next->deca[i];
            ukupan_broj_koraka++;
        }
        for (i = 0; i < next->n; i++) {
            ukupan_broj_koraka++;
            if (next->keys[i] == kljucevi[br]) {
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "Nema kljuca " << kljucevi[br] << " u stablu!\n";
            cout << "Broj koraka: " << ukupan_broj_koraka << endl;
        } else {
            Zapis *podaci = next->podaci[i];
            f_out << 43000000000 + kljucevi[br] << "|";
            podaci->ispis(f_out);
            cout << "Kljuc " << kljucevi[br] << " je pronadjen u " << ukupan_broj_koraka << " koraka." << endl;
        }
        br++;
    }
    int j = i;                            //indeks elementa koji je pronadjen pre njega
    for (i = br; i < k; i++) {
        cout << "DA\n";
        bool done = false;                //indikator da smo ili nasli, ili nema tog kljuca (jer su sortirani)
        while (!done) {
            //cout << kljucev << endl;
            while (j < next->n && kljucevi[i] > next->keys[j]) {
                ukupan_broj_koraka++;
                j++;
            }
            if (j == next->n) {                             //prelazak na sledeci cvor
                //cout << j << endl;
                next = next->deca[m - 1];
                j = 0;
            } else {
                done = true;
                if (kljucevi[i] == next->keys[j]) {
                    cout << kljucevi[i] << endl;
                    Zapis *podaci = next->podaci[j];
                    f_out << 43000000000 + kljucevi[i] << "|";
                    podaci->ispis(f_out);
                    cout << "Kljuc " << kljucevi[i] << " je pronadjen u " << ukupan_broj_koraka << " koraka." << endl;
                } else {
                    cout << "Nema kljuca " << kljucevi[i] << "u stablu!\n";
                    cout << "Broj koraka: " << ukupan_broj_koraka << endl;
                }
            }
        }
    }
    cout << "Pretraga je zavrsena u ukupno " << ukupan_broj_koraka << " koraka." << endl << endl;
    return ukupan_broj_koraka;
}

Cvor *read_file() {
    int num;
    string name;
    cout << "Unesite kolicinu podataka u fajlu iz kog se citaju podaci(20,250,2500): ";
    cin >> num;
    if (num == 20) {
        name = "CustomerAccount20.txt";
    } else if (num == 250) {
        name = "CustomerAccount250.txt";
    } else {
        name = "CustomerAccount2500.txt";
    }
    ifstream file(name);
    while (!file.is_open()) {
        cout << "Ne postoji datoteka koju trazite";
        exit(1);
    }

    Cvor *root = nullptr;
    string line;
    while (getline(file, line)) {
        char *data = nullptr;        //pojedinacan podatak upisujem u promenljivu data
        int br_rec = 0, br_znak = 0;
        int key, b, c, tax, tren;
        string ime, balans;

        for (char elem: line) {
            br_znak++;                                  //znak je za 1 veci jer citamo sledeci karakter
            data = (char *) realloc(data, br_znak + 1);      //realnociranje prostora za novi karakter
            if (elem == '|' || elem == '\n') {
                if (br_rec < 3) {
                    string x = new char[br_znak - 2];
                    for (int i = 0; i < br_znak - 2; i++)
                        x[i] = data[i + 2];   //jer su prve dve cifre nepotrebne (mnogo je velik broj)
                    tren = stoi(x);
                    switch (br_rec) {
                        case 0:
                            key = tren;
                        case 1:
                            b = tren;
                        case 2:
                            c = tren;
                    }
                } else if (br_rec == 3) ime = data;
                else if (br_rec == 4) tax = stoi(data);
                br_rec++;           //prebacuje se na sledecu rec u redu
                br_znak = 0;          //pocetak nove reci
                data = nullptr;       //citanje nove reci
            } else {
                data[br_znak - 1] = elem;   //ukoliko nije nova rec smao se dodaje na kraj promenljive data
                data[br_znak] = '\0';
            }
        }
        balans = data;                //poslednji clan reda je ostao da se upise
        Zapis *podaci = new Zapis(b, c, tax, ime, balans);
        root = umetanje_u_stablo(root, podaci, key);
        cout << "root " << root->keys[0] << endl;
        //LEVEL_ORDER(root);
    }
    file.close();                           //obavezno zatvaranje datoteke
    return root;
}

void meni() {
    cout << "Opcije: \n"
            "1. Stvaranje B+ stabla na osnovu tabele\n"
            "2. Ispis stabla\n"
            "3. Dodavanje zapisa u stablo\n"
            "4. Brisanje zapisa iz stabla\n"
            "5. Pretrazivanje jednog podatka\n"
            "6. Pretrazivanje k podataka\n"
            "7. Pretrazivanje k sukcesivnih podataka\n"
            "8. Brisanje stabla\n"
            "9. Zavrsavanje programa\n";
}

Zapis *citanjeZapisa() {
    int b, c, tax;
    string ime, balans;
    cout << "Unesite 2 cela broja: CA_B_ID i CA_C_ID: ";
    cin >> b >> c;
    cout << "Unesite naziv zapisa: ";
    cin >> ime;
    cout << "Unesite CA_TAX_ST: ";
    cin >> tax;
    cout << "Unesite balans: ";
    cin >> balans;
    return new Zapis(b, c, tax, ime, balans);
}

int main() {
    int opcija;
    meni();
    do {
        cout << "Izaberite jednu od ponudjenih opcija: ";
        cin >> opcija;
    } while (opcija < 1 || opcija > 9);

    Cvor *root = nullptr;
    while (opcija != 9) {
        if (opcija == 1) {
            root = read_file();
        } else if (opcija == 2) {
            LEVEL_ORDER(root);
        } else if (opcija == 3) {
            int key;
            cout << "Unesite kljuc koji se ubacuje: ";
            cin >> key;
            Zapis *z = citanjeZapisa();
            root = umetanje_u_stablo(root, z, key);
        } else if (opcija == 4) {
            int key;
            cout << "Unesite kljuc koji se brise: ";
            cin >> key;
            root = brisanje(root, key);
        } else if (opcija == 5) {
            int key;
            cout << "Unesite kljuc koji se brise: ";
            cin >> key;
            pretraga(root, key);
        } else if (opcija == 6) {
            visestruka_pretraga(root);
        } else if (opcija == 7) {
            sukcesivna_pretraga(root);
        } else if (opcija == 8) {
            delete root;
            root = nullptr;
        }

        meni();
        do {
            cout << "Izaberite jednu od ponudjenih opcija: ";
            cin >> opcija;
        } while (opcija < 1 || opcija > 9);
    }
    return 0;
}