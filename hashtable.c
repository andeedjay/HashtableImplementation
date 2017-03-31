#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 20000

typedef struct node {
	const char *value;
	struct node *next;
} *Node, **Hashtable;

const char *myStrdup(const char *str);
Node newNode(const char *word);
void addWord(Hashtable ht, unsigned int dim, const char *word);
void removeWord(Hashtable ht, unsigned int dim, const char *word);
int findWord(Hashtable ht, unsigned int dim, const char *word);
void printBucket(Hashtable ht, unsigned int indexBucket, FILE *out);
void print(Hashtable ht, unsigned int dim, FILE *out);
Hashtable resize(Hashtable ht, unsigned int *dim, char *mode);

void freeList(Node head);
void clear(Hashtable ht, unsigned int dim);
char **parseCommand(char buffer[BUFSIZE], unsigned int *noWords);
int commandNameIsValid(char *command);
int argumentsAreValid(char **words, unsigned int noWords, unsigned int dim);
void execute(char **words, unsigned int noWords,
Hashtable *ht, unsigned int *dim);
void freeWords(char **words, int noWords);

int main(int argc, char *argv[])
{
	unsigned int SIZE;
	Hashtable ht;
	char buffer[BUFSIZE];
	FILE **in;
	unsigned int i, j, k;
	unsigned int noInputs, noWords;
	char **words;
	char error[BUFSIZE];

	/*nu se da dimensiunea tabelei => eroare*/
	if (argc < 2) {
		strcpy(error, "Usage: ");
		strcat(error, argv[0]);
		strcat(error, " size [input1] [input2] [...]");
		fprintf(stderr, "%s\n", error);
		exit(1);
	}

	/* daca dimensiunea primita in linia de comanda*/
	/*e negativa sau nu e numar (ci string) */
	SIZE = atoi(argv[1]);
	if ((int) SIZE <= 0) {
		strcpy(error, "First command line argument ");
		strcat(error, "should be an unsigned int");
		fprintf(stderr, "%s\n", error);
		exit(1);
	}

	ht = calloc(SIZE, sizeof(Node));

	/*daca nu se specifica fisier/e de intrare*/
	/*citirea se va face de la stdin*/
	if (argc == 2) {
		in = malloc(sizeof(FILE *));
		in[0] = stdin;
		noInputs = 1;
	} else if (argc > 2) {
		noInputs = argc - 2;
		in = malloc(noInputs * sizeof(FILE *));
		for (i = 0; i < noInputs; i++)
			in[i] = fopen(argv[i + 2], "r");
	}

	for (i = 0; i < noInputs; i++) {
		while (fgets(buffer, BUFSIZE, in[i]) != NULL) {

			/*sar peste liniile goale*/
			if (strcmp(buffer, "\n") != 0) {
				noWords = 0;
				/*parsez comanda intr-un vector de cuvinte*/
				words = parseCommand(buffer, &noWords);
				/*verific daca numele comenzii e valid*/
				if (commandNameIsValid(words[0])) {
					/*daca argumentele comenzii ele valide*/
					if (argumentsAreValid(words,
							noWords, SIZE) == 1) {
						/*execut comanda*/
						execute(words, noWords,
							&ht, &SIZE);
						/*eliberez memoria folosita*/
						/*pentrua salva comanda si*/
						/*argumentele sale*/
						freeWords(words, noWords);
					} else {
						k = i;
						goto exit;
					}
				} else {
					strcpy(error, "Unrecognized ");
					strcat(error, "command name");
					fprintf(stderr, "%s\n", error);
					k = i;
					goto exit;
				}
			}
		}
		fclose(in[i]);
	}
	free(in);
	clear(ht, SIZE);
	free(ht);
	return 0;

exit:
	clear(ht, SIZE);
	free(ht);
	for (i = 0; i < noWords; i++)
		free(words[i]);
	free(words);
	for (i = k; i < noInputs; i++)
		fclose(in[i]);
	free(in);
}

/*functie echivalenta strdup, dar care*/
/*returneaza  const char* in loc de char* */
const char *myStrdup(const char *str)
{
	char *newstr = malloc((strlen(str) + 1) * sizeof(char));

	if (newstr == NULL)
		return NULL;
	strcpy(newstr, str);
	return newstr;
}

/*functie de alocare a unui nou nod*/
Node newNode(const char *word)
{
	Node newWord;

	newWord = malloc(sizeof(struct node));
	newWord->value = myStrdup(word);
	newWord->next = NULL;
	return newWord;
}

/*functie de adaugare a unui cuvant in hashtable*/
void addWord(Hashtable ht, unsigned int dim, const char *word)
{
	/*vad la ce index din tabela trebuie sa fac inserarea*/
	unsigned int key = hash(word, dim);
	Node head = ht[key];
	Node temp = head;
	Node aux = NULL;

	/*am grija sa nu inserez cuvinte goale*/
	if (word == NULL)
		return;

	/*cand capul listei in care vreau sa inserez cuvantul*/
	/*e gol fac un nou nod cu valoarea de inserat*/
	/*si il pun in tabela de disperia la indexul gasit*/
	if (head == NULL) {
		temp = newNode(word);
		ht[key] = temp;
	} else {
		/*altfel iterez prin lista si daca nu gasesc cuvantul*/
		/*deja inserat il inserez la sfarsitul listei*/
		while (head != NULL) {
			if (strcmp(head->value, word) == 0)
				return;
			aux = head;
			head = head->next;
		}
		if (aux->next == NULL)
			aux->next = newNode(word);
	}
}

/*functie de stergere a unui cuvant din hashtable*/
void removeWord(Hashtable ht, unsigned int dim, const char *word)
{
	/*vad la ce index din tabela ar trebui sa gasesc cuvantul*/
	/*pe care vreau sa il sterg si salvez capul listei*/
	unsigned int key = hash(word, dim);
	Node head = ht[key];
	Node temp = head;
	Node prev = NULL;

	/*cuvantul nu exista in tabela sau este gol*/
	if (head == NULL || word == NULL)
		return;
	/*cuvantul e chiar capul listei fac capul urmatorul nod*/
	/*din lista si eliberez memoria pentru capul initial*/
	else if (strcmp(head->value, word) == 0) {
		temp = head->next;
		free((char *) head->value);
		free(head);
	} else {
		/*iterez prin lista pana gasesc nodul cu cuvantul cautat*/
		/*si leg nodul precedent nodului gasit cu nodul de dupa nodul*/
		/*gasit si eliberez memoria alocata nodului curent*/
		temp = head;
		prev = NULL;
		while (head != NULL) {
			if (strcmp(head->value, word) == 0) {
				prev->next = head->next;
				free((char *) head->value);
				free(head);
				break;
			}
			prev = head;
			head = head->next;
		}
	}
	ht[key] = temp;
}

/*functia de cautare intr-un hashtable*/
int findWord(Hashtable ht, unsigned int dim, const char *word)
{
	/*caut intai indicele la care ar trebui sa se gaseasca cuvantul*/
	unsigned int key = hash(word, dim);
	Node head = ht[key];

	/*iterez prin lista de la indicele gasit mai sus si il caut*/
	while (head != NULL) {
		if (strcmp(head->value, word) == 0)
			return 1;
		head = head->next;
	}
	return 0;
}

/*functie de eliberare a memoriei pentru nodurile unei liste*/
void freeList(Node head)
{
	Node temp;

	while (head != NULL) {
		temp = head;
		head = head->next;
		free((char *) temp->value);
		free(temp);
	}
}

/*functie de stergere a tuturor elementelor din hashtable*/
void clear(Hashtable ht, unsigned int dim)
{
	unsigned int i;
	Node head;

	for (i = 0; i < dim; i++) {
		head = ht[i];
		freeList(head);
		ht[i] = NULL;
	}
}

/*functie ce printeaza o linie din hashtable*/
void printBucket(Hashtable ht, unsigned int indexBucket, FILE *out)
{
	Node head = ht[indexBucket];

	while (head != NULL) {
		fprintf(out, "%s ", head->value);
		head = head->next;
	}
	fprintf(out, "\n");
}

/*functie ce printeaza intreg hashtable-ul linie sub linie*/
void print(Hashtable ht, unsigned int dim, FILE *out)
{
	int i;
	Node head;

	for (i = 0; i < dim; i++) {
		head = ht[i];
		while (head != NULL) {
			fprintf(out, "%s ", head->value);
			head = head->next;
		}
		if (ht[i] != NULL)
			fprintf(out, "\n");
	}
}

/*functie de redimensioneaza tabela de dispersie*/
Hashtable resize(Hashtable ht, unsigned int *dim, char *mode)
{
	unsigned int newDim, i;
	Hashtable newHT;
	Node head;

	/*functie de mod dublam sau injumatatim dimensiunea*/
	if (strcmp(mode, "double") == 0)
		newDim = (*dim) * 2;
	else if (strcmp(mode, "halve") == 0)
		newDim = (*dim) / 2;

	/*alocam memorie pentru alt hashtable cu noua dimensiune*/
	newHT = calloc(newDim, sizeof(struct node));
	/*pentru fiecare linie (lista) din hashtable*/
	for (i = 0; i < *dim; i++) {
		head = ht[i];
		/*parcurg toate liniile si inserez fiecare cuvant*/
		/*de pe linia curenta in noul hashtable*/
		while (head != NULL) {
			addWord(newHT, newDim, head->value);
			head = head->next;
		}
	}
	/*eliberez memoria alocata vechiului hashtable*/
	clear(ht, *dim);
	free(ht);
	*dim = newDim;
	return newHT;
}

/*functie ce parseaza comanda primita de la stdin sau din fisier*/
/*imparte comanda dupa spatii sau '\n' si intoarce un array de cuvinte*/
char **parseCommand(char buffer[BUFSIZE], unsigned int *noWords)
{
	unsigned int i;
	const char *sep = "\n ";
	char *token;
	char **words;

	*noWords = 1;
	for (i = 0; i < strlen(buffer); i++)
		if (buffer[i] == ' ')
			(*noWords)++;
	token = strtok(buffer, sep);
	words = malloc((*noWords) * sizeof(char *));
	while (token != NULL) {
		*words = malloc((strlen(token) + 1) * sizeof(char));
		strcpy(*words, token);
		words++;
		token = strtok(NULL, sep);
	}
	words -= (*noWords);
	return words;
}

/*functie ce verifica daca numele comenzii primite e valid*/
int commandNameIsValid(char *command)
{
	return (strcmp(command, "add") == 0) ||
		(strcmp(command, "remove") == 0) ||
		(strcmp(command, "find") == 0) ||
		(strcmp(command, "clear") == 0) ||
		(strcmp(command, "print_bucket") == 0) ||
		(strcmp(command, "print") == 0) ||
		(strcmp(command, "resize") == 0);
}

/*functie ce verifica daca argumentele unei comenzi sunt*/
/*valide; verificarea se face mai ales dupa numarul*/
/*si tipul lor, dar si dupa valoaree (ex: print_bucket)*/
int argumentsAreValid(char **words, unsigned int noWords, unsigned int dim)
{
	char error[BUFSIZE];
	char missingArg[BUFSIZE] = "Missing argument for";
	char manyArgs[BUFSIZE] = "Too many arguments for";

	if (strcmp(words[0], "add") == 0 || strcmp(words[0], "remove") == 0) {
		if (noWords == 1) {
			fprintf(stderr, "%s %s command\n",
				missingArg, words[0]);
			return -1;
		} else if (noWords > 2) {
			fprintf(stderr, "%s %s command\n",
				manyArgs, words[0]);
			return -1;
		} else if (noWords == 2)
			return 1;
	} else if (strcmp(words[0], "find") == 0) {
		if (noWords == 1) {
			fprintf(stderr, "%s %s command\n",
				missingArg, words[0]);
			return -1;
		} else if (noWords > 3) {
			fprintf(stderr, "%s %s command\n",
				manyArgs, words[0]);
			return -1;
		} else if (noWords == 2 || noWords == 3)
			return 1;
		} else if (strcmp(words[0], "clear") == 0) {
			if (noWords > 1) {
				fprintf(stderr, "%s %s command\n",
					manyArgs, words[0]);
				return -1;
			} else if (noWords == 1)
				return 1;
		} else if (strcmp(words[0], "print_bucket") == 0) {
			if (noWords == 1) {
				fprintf(stderr, "%s %s command\n",
					missingArg, words[0]);
				return -1;
			} else if (noWords > 3) {
				fprintf(stderr, "%s %s command\n",
					manyArgs, words[0]);
				return -1;
			} else if ((noWords == 2 || noWords == 3) &&
					   (atoi(words[1]) > 0 ||
					    strcmp(words[1], "0") == 0) &&
					   (atoi(words[1]) < dim)) {
				return 1;
			} else if ((noWords == 2 || noWords == 3) &&
					   (atoi(words[1]) <= 0) ||
					    atoi(words[1]) >= dim) {
				strcpy(error, "Second argument should ");
				strcat(error, "be an unsigned in less ");
				strcat(error, "than hashtable's size");
				fprintf(stderr, "%s\n", error);
				return -1;
			}
		} else if (strcmp(words[0], "print") == 0) {
			if (noWords > 2) {
				fprintf(stderr, "%s %s command\n",
					manyArgs, words[0]);
				return -1;
			} else if (noWords == 1 || noWords == 2)
				return 1;
		} else if (strcmp(words[0], "resize") == 0) {
			if (noWords == 1) {
				fprintf(stderr, "%s %s command\n",
					missingArg, words[0]);
				return -1;
			} else if (noWords > 2) {
				fprintf(stderr, "%s %s command\n",
					manyArgs, words[0]);
				return -1;
			} else if (strcmp(words[1], "double") == 0 ||
					   strcmp(words[1], "halve") == 0)
				return 1;
			strcpy(error, "Wrong mode of resizing");
			strcat(error, " the hashtable");
			fprintf(stderr, "%s\n", error);
			return -1;
		}
	return -1;
}

/*functia de executare a unei comenzi ce primeste comanda si*/
/*argumentele ei, avand ca rezultat modificarea datelor din hashtable*/
/*sau hashtable-ul in sine (ex: resize cand se modifica dimensiunea lui)*/
void execute(char **words, unsigned int noWords,
	Hashtable *ht, unsigned int *dim)
{
	int found;
	FILE *out;

	if (strcmp(words[0], "add") == 0) {
		addWord(*ht, *dim, words[1]);
		return;
	} else if (strcmp(words[0], "remove") == 0) {
		removeWord(*ht, *dim, words[1]);
		return;
	} else if (strcmp(words[0], "find") == 0) {
		found = findWord(*ht, *dim, words[1]);
		if (noWords == 2) {
			if (found == 1)
				fprintf(stdout, "True\n");
			else
				fprintf(stdout, "False\n");
		} else if (noWords == 3) {
			out = fopen(words[2], "a");
			if (found == 1)
				fprintf(out, "True\n");
			else
				fprintf(out, "False\n");
			fclose(out);
		}
		return;
	} else if (strcmp(words[0], "clear") == 0) {
		clear(*ht, *dim);
		return;
	} else if (strcmp(words[0], "print_bucket") == 0) {
		if (noWords == 2)
			printBucket(*ht, atoi(words[1]), stdout);
		else if (noWords == 3) {
			out = fopen(words[2], "a");
			printBucket(*ht, atoi(words[1]), out);
			fclose(out);
		}
		return;
	} else if (strcmp(words[0], "print") == 0) {
		if (noWords == 1)
			print(*ht, *dim, stdout);
		else if (noWords == 2) {
			out = fopen(words[1], "a");
			print(*ht, *dim, out);
			fclose(out);
		}
		return;
	} else if (strcmp(words[0], "resize") == 0)
		*ht = resize(*ht, dim, words[1]);
}

/*functie ce elibereaza memoria pentru un vector de cuvinte*/
/*folosesc unul pentru a salva comanda si argumentele ei*/
void freeWords(char **words, int noWords)
{
	unsigned int j;

	for (j = 0; j < noWords; j++)
		free(words[j]);
	free(words);
}
