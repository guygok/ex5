/***********
 ID: kejzmag
 NAME: Guy Kejzman
***********/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- Defines (no magic numbers) ---------- */
#define START_CAP 16
#define TIME_LEN 8
#define COLON1_POS 2
#define COLON2_POS 5
#define MAX_HH 99
#define MAX_MM 59
#define MAX_SS 59
#define TWO_DIGITS 2
#define TEN 10

typedef struct Episode {
	char* name;
	char* length;
	struct Episode* next;
} Episode;

typedef struct Season {
	char* name;
	Episode* episodes;
	struct Season* next;
} Season;

typedef struct TVShow {
	char* name;
	Season* seasons;
} TVShow;

/* ---------- Globals (only allowed ones) ---------- */
TVShow*** database = NULL;
int dbSize = 0;

/* ---------- Prototypes (style requirement) ---------- */
static char* dupStr(const char* s);
char* getString(void);
int getInt(void);
int isValidLength(const char* s);

int countShows(void);

static void initDB(int n);
void expandDB(void);
void shrinkDB(void);

void freeEpisode(Episode* ep);
void freeSeason(Season* sn);
void freeShow(TVShow* show);
void freeAll(void);

TVShow* findShow(const char* showName);
Season* findSeason(TVShow* show, const char* seasonName);
Episode* findEpisode(Season* season, const char* epName);

static TVShow* getAt(int idx);
static void setAt(int idx, TVShow* show);
static int packedCount(void);
static int findInsertSpot(const char* showName);

void addShow(void);
void addSeason(void);
void addEpisode(void);

void deleteShow(void);
void deleteSeason(void);
void deleteEpisode(void);

void printEpisode(void);
void printShow(void);
void printArray(void);

void addMenu(void);
void deleteMenu(void);
void printMenuSub(void);
void mainMenu(void);

/* ---------- Helpers ---------- */

/* Duplicate a string (malloc + copy). */
static char* dupStr(const char* s) {
	size_t n = strlen(s);
	char* out = (char*)malloc(n + 1);
	size_t i = 0;

	if (out == NULL) {
		exit(1);
	}

	for (i = 0; i <= n; i++) {
		out[i] = s[i];
	}

	return out;
}

/* Read a whole line until '\n' or EOF. Returns a malloc'ed string. */
char* getString(void) {
	int ch = 0;
	size_t cap = START_CAP;
	size_t len = 0;
	char* txt = (char*)malloc(cap);

	if (txt == NULL) {
		exit(1);
	}

	while ((ch = getchar()) != '\n' && ch != EOF) {
		if (len + 1 >= cap) {
			char* bigger = NULL;

			cap *= 2;
			bigger = (char*)realloc(txt, cap);
			if (bigger == NULL) {
				free(txt);
				exit(1);
			}
			txt = bigger;
		}
		txt[len] = (char)ch;
		len++;
	}

	txt[len] = '\0';
	return txt;
}

int getInt(void) {
	char* txt = getString();
	int val = atoi(txt);

	free(txt);
	return val;
}

int isValidLength(const char* s) {
	int hh = 0;
	int mm = 0;
	int ss = 0;
	int i = 0;

	if (s == NULL) {
		return 0;
	}

	if (strlen(s) != TIME_LEN) {
		return 0;
	}

	if (s[COLON1_POS] != ':' || s[COLON2_POS] != ':') {
		return 0;
	}

	for (i = 0; i < TIME_LEN; i++) {
		if (i == COLON1_POS || i == COLON2_POS) {
			continue;
		}
		if (s[i] < '0' || s[i] > '9') {
			return 0;
		}
	}

	hh = (s[0] - '0') * TEN + (s[1] - '0');
	mm = (s[3] - '0') * TEN + (s[4] - '0');
	ss = (s[6] - '0') * TEN + (s[7] - '0');

	if (hh < 0 || hh > MAX_HH) {
		return 0;
	}
	if (mm < 0 || mm > MAX_MM) {
		return 0;
	}
	if (ss < 0 || ss > MAX_SS) {
		return 0;
	}

	return 1;
}

/* ---------- DB helpers ---------- */

int countShows(void) {
	int r = 0;
	int c = 0;
	int cnt = 0;

	if (database == NULL || dbSize == 0) {
		return 0;
	}

	for (r = 0; r < dbSize; r++) {
		for (c = 0; c < dbSize; c++) {
			if (database[r][c] != NULL) {
				cnt++;
			}
		}
	}

	return cnt;
}

static void initDB(int n) {
	int r = 0;
	int c = 0;

	dbSize = n;

	if (n == 0) {
		database = NULL;
		return;
	}

	database = (TVShow***)malloc(sizeof(*database) * (size_t)n);
	if (database == NULL) {
		exit(1);
	}

	for (r = 0; r < n; r++) {
		database[r] = (TVShow**)malloc(sizeof(*database[r]) * (size_t)n);
		if (database[r] == NULL) {
			exit(1);
		}
		for (c = 0; c < n; c++) {
			database[r][c] = NULL;
		}
	}
}

void expandDB(void) {
	int oldN = dbSize;
	int newN = oldN + 1;
	int r = 0;
	int c = 0;
	int k = 0;
	TVShow*** oldDb = database;

	initDB(newN);

	if (oldDb == NULL || oldN == 0) {
		return;
	}

	for (r = 0; r < oldN; r++) {
		for (c = 0; c < oldN; c++) {
			if (oldDb[r][c] != NULL) {
				int nr = k / newN;
				int nc = k % newN;

				database[nr][nc] = oldDb[r][c];
				k++;
			}
		}
	}

	for (r = 0; r < oldN; r++) {
		free(oldDb[r]);
	}
	free(oldDb);
}

void shrinkDB(void) {
	int oldN = dbSize;
	int newN = oldN - 1;
	int r = 0;
	int c = 0;
	int k = 0;
	int showCount = 0;
	TVShow*** oldDb = database;

	if (dbSize <= 1) {
		if (dbSize == 1 && database != NULL && database[0][0] == NULL) {
			free(database[0]);
			free(database);
			database = NULL;
			dbSize = 0;
		}
		return;
	}

	showCount = countShows();
	if (showCount > newN * newN) {
		return;
	}

	initDB(newN);

	for (r = 0; r < oldN; r++) {
		for (c = 0; c < oldN; c++) {
			if (oldDb[r][c] != NULL) {
				int nr = k / newN;
				int nc = k % newN;

				database[nr][nc] = oldDb[r][c];
				k++;
			}
		}
	}

	for (r = 0; r < oldN; r++) {
		free(oldDb[r]);
	}
	free(oldDb);
}

/* ---------- Freeing ---------- */

void freeEpisode(Episode* ep) {
	while (ep != NULL) {
		Episode* nextOne = ep->next;

		free(ep->name);
		free(ep->length);
		free(ep);

		ep = nextOne;
	}
}

void freeSeason(Season* sn) {
	while (sn != NULL) {
		Season* nextOne = sn->next;

		free(sn->name);
		freeEpisode(sn->episodes);
		free(sn);

		sn = nextOne;
	}
}

void freeShow(TVShow* show) {
	if (show == NULL) {
		return;
	}

	free(show->name);
	freeSeason(show->seasons);
	free(show);
}

void freeAll(void) {
	int r = 0;
	int c = 0;

	if (database == NULL || dbSize == 0) {
		return;
	}

	for (r = 0; r < dbSize; r++) {
		for (c = 0; c < dbSize; c++) {
			if (database[r][c] != NULL) {
				freeShow(database[r][c]);
				database[r][c] = NULL;
			}
		}
	}

	for (r = 0; r < dbSize; r++) {
		free(database[r]);
	}
	free(database);

	database = NULL;
	dbSize = 0;
}

/* ---------- Finders ---------- */

TVShow* findShow(const char* showName) {
	int r = 0;
	int c = 0;

	if (database == NULL || dbSize == 0) {
		return NULL;
	}

	for (r = 0; r < dbSize; r++) {
		for (c = 0; c < dbSize; c++) {
			TVShow* s = database[r][c];

			if (s != NULL && strcmp(s->name, showName) == 0) {
				return s;
			}
		}
	}

	return NULL;
}

Season* findSeason(TVShow* show, const char* seasonName) {
	Season* cur = NULL;

	if (show == NULL) {
		return NULL;
	}

	cur = show->seasons;
	while (cur != NULL) {
		if (strcmp(cur->name, seasonName) == 0) {
			return cur;
		}
		cur = cur->next;
	}

	return NULL;
}

Episode* findEpisode(Season* season, const char* epName) {
	Episode* cur = NULL;

	if (season == NULL) {
		return NULL;
	}

	cur = season->episodes;
	while (cur != NULL) {
		if (strcmp(cur->name, epName) == 0) {
			return cur;
		}
		cur = cur->next;
	}

	return NULL;
}

/* ---------- Packed array helpers ---------- */

static TVShow* getAt(int idx) {
	int r = idx / dbSize;
	int c = idx % dbSize;

	return database[r][c];
}

static void setAt(int idx, TVShow* show) {
	int r = idx / dbSize;
	int c = idx % dbSize;

	database[r][c] = show;
}

static int packedCount(void) {
	return countShows();
}

static int findInsertSpot(const char* showName) {
	int i = 0;
	int n = packedCount();

	for (i = 0; i < n; i++) {
		TVShow* s = getAt(i);

		if (s != NULL && strcmp(showName, s->name) < 0) {
			return i;
		}
	}

	return n;
}

/* ---------- Add ---------- */

void addShow(void) {
	char* showName = NULL;
	TVShow* newShow = NULL;
	int howMany = 0;
	int spot = 0;
	int i = 0;

	printf("Enter the name of the show:\n");
	showName = getString();

	if (findShow(showName) != NULL) {
		printf("Show already exists.\n");
		free(showName);
		return;
	}

	if (dbSize == 0) {
		initDB(1);
	}
	else {
		howMany = countShows();
		if (howMany == dbSize * dbSize) {
			expandDB();
		}
	}

	newShow = (TVShow*)malloc(sizeof(*newShow));
	if (newShow == NULL) {
		exit(1);
	}
	newShow->name = dupStr(showName);
	newShow->seasons = NULL;

	spot = findInsertSpot(showName);
	free(showName);

	howMany = packedCount();

	for (i = howMany; i > spot; i--) {
		setAt(i, getAt(i - 1));
	}
	setAt(spot, newShow);
}

void addSeason(void) {
	char* showName = NULL;
	char* seasonName = NULL;
	TVShow* showPtr = NULL;
	Season* newSeason = NULL;
	Season* cur = NULL;
	int pos = 0;
	int i = 0;

	printf("Enter the name of the show:\n");
	showName = getString();
	showPtr = findShow(showName);

	if (showPtr == NULL) {
		printf("Show not found.\n");
		free(showName);
		return;
	}

	printf("Enter the name of the season:\n");
	seasonName = getString();

	if (findSeason(showPtr, seasonName) != NULL) {
		printf("Season already exists.\n");
		free(showName);
		free(seasonName);
		return;
	}

	printf("Enter the position:\n");
	pos = getInt();

	newSeason = (Season*)malloc(sizeof(*newSeason));
	if (newSeason == NULL) {
		exit(1);
	}
	newSeason->name = dupStr(seasonName);
	newSeason->episodes = NULL;
	newSeason->next = NULL;

	free(showName);
	free(seasonName);

	if (pos == 0 || showPtr->seasons == NULL) {
		newSeason->next = showPtr->seasons;
		showPtr->seasons = newSeason;
		return;
	}

	cur = showPtr->seasons;
	while (cur->next != NULL && i < pos - 1) {
		cur = cur->next;
		i++;
	}

	newSeason->next = cur->next;
	cur->next = newSeason;
}

void addEpisode(void) {
	char* showName = NULL;
	char* seasonName = NULL;
	char* epName = NULL;
	char* timeStr = NULL;

	TVShow* showPtr = NULL;
	Season* seasonPtr = NULL;
	Episode* newEp = NULL;
	Episode* cur = NULL;

	int pos = 0;
	int i = 0;

	printf("Enter the name of the show:\n");
	showName = getString();
	showPtr = findShow(showName);

	if (showPtr == NULL) {
		printf("Show not found.\n");
		free(showName);
		return;
	}

	printf("Enter the name of the season:\n");
	seasonName = getString();
	seasonPtr = findSeason(showPtr, seasonName);

	if (seasonPtr == NULL) {
		printf("Season not found.\n");
		free(showName);
		free(seasonName);
		return;
	}

	printf("Enter the name of the episode:\n");
	epName = getString();

	if (findEpisode(seasonPtr, epName) != NULL) {
		printf("Episode already exists.\n");
		free(showName);
		free(seasonName);
		free(epName);
		return;
	}

	printf("Enter the length (xx:xx:xx):\n");
	timeStr = getString();
	while (isValidLength(timeStr) == 0) {
		printf("Invalid length, enter again:\n");
		free(timeStr);
		timeStr = getString();
	}

	printf("Enter the position:\n");
	pos = getInt();

	newEp = (Episode*)malloc(sizeof(*newEp));
	if (newEp == NULL) {
		exit(1);
	}
	newEp->name = dupStr(epName);
	newEp->length = dupStr(timeStr);
	newEp->next = NULL;

	free(showName);
	free(seasonName);
	free(epName);
	free(timeStr);

	if (pos == 0 || seasonPtr->episodes == NULL) {
		newEp->next = seasonPtr->episodes;
		seasonPtr->episodes = newEp;
		return;
	}

	cur = seasonPtr->episodes;
	while (cur->next != NULL && i < pos - 1) {
		cur = cur->next;
		i++;
	}

	newEp->next = cur->next;
	cur->next = newEp;
}

/* ---------- Delete ---------- */

void deleteShow(void) {
	char* showName = NULL;
	int n = 0;
	int idx = -1;
	int i = 0;
	int showCount = 0;

	printf("Enter the name of the show:\n");
	showName = getString();

	if (database == NULL || dbSize == 0) {
		printf("Show not found.\n");
		free(showName);
		return;
	}

	n = packedCount();
	for (i = 0; i < n; i++) {
		TVShow* s = getAt(i);

		if (s != NULL && strcmp(s->name, showName) == 0) {
			idx = i;
			break;
		}
	}

	if (idx == -1) {
		printf("Show not found.\n");
		free(showName);
		return;
	}

	free(showName);

	freeShow(getAt(idx));

	for (i = idx; i < n - 1; i++) {
		setAt(i, getAt(i + 1));
	}
	setAt(n - 1, NULL);

	showCount = countShows();
	if (dbSize > 1 && showCount <= (dbSize - 1) * (dbSize - 1)) {
		shrinkDB();
	}
	else if (dbSize == 1 && showCount == 0) {
		shrinkDB();
	}
}

void deleteSeason(void) {
	char* showName = NULL;
	char* seasonName = NULL;

	TVShow* showPtr = NULL;
	Season* cur = NULL;
	Season* prev = NULL;

	printf("Enter the name of the show:\n");
	showName = getString();
	showPtr = findShow(showName);

	if (showPtr == NULL) {
		printf("Show not found.\n");
		free(showName);
		return;
	}

	printf("Enter the name of the season:\n");
	seasonName = getString();

	cur = showPtr->seasons;
	while (cur != NULL && strcmp(cur->name, seasonName) != 0) {
		prev = cur;
		cur = cur->next;
	}

	if (cur == NULL) {
		printf("Season not found.\n");
		free(showName);
		free(seasonName);
		return;
	}

	if (prev == NULL) {
		showPtr->seasons = cur->next;
	}
	else {
		prev->next = cur->next;
	}

	cur->next = NULL;
	freeSeason(cur);

	free(showName);
	free(seasonName);
}

void deleteEpisode(void) {
	char* showName = NULL;
	char* seasonName = NULL;
	char* epName = NULL;

	TVShow* showPtr = NULL;
	Season* seasonPtr = NULL;
	Episode* cur = NULL;
	Episode* prev = NULL;

	printf("Enter the name of the show:\n");
	showName = getString();
	showPtr = findShow(showName);

	if (showPtr == NULL) {
		printf("Show not found.\n");
		free(showName);
		return;
	}

	printf("Enter the name of the season:\n");
	seasonName = getString();
	seasonPtr = findSeason(showPtr, seasonName);

	if (seasonPtr == NULL) {
		printf("Season not found.\n");
		free(showName);
		free(seasonName);
		return;
	}

	printf("Enter the name of the episode:\n");
	epName = getString();

	cur = seasonPtr->episodes;
	while (cur != NULL && strcmp(cur->name, epName) != 0) {
		prev = cur;
		cur = cur->next;
	}

	if (cur == NULL) {
		printf("Episode not found.\n");
		free(showName);
		free(seasonName);
		free(epName);
		return;
	}

	if (prev == NULL) {
		seasonPtr->episodes = cur->next;
	}
	else {
		prev->next = cur->next;
	}

	cur->next = NULL;
	freeEpisode(cur);

	free(showName);
	free(seasonName);
	free(epName);
}

/* ---------- Print ---------- */

void printEpisode(void) {
	char* showName = NULL;
	char* seasonName = NULL;
	char* epName = NULL;

	TVShow* showPtr = NULL;
	Season* seasonPtr = NULL;
	Episode* epPtr = NULL;

	printf("Enter the name of the show:\n");
	showName = getString();
	showPtr = findShow(showName);

	if (showPtr == NULL) {
		printf("Show not found.\n");
		free(showName);
		return;
	}

	printf("Enter the name of the season:\n");
	seasonName = getString();
	seasonPtr = findSeason(showPtr, seasonName);

	if (seasonPtr == NULL) {
		printf("Season not found.\n");
		free(showName);
		free(seasonName);
		return;
	}

	printf("Enter the name of the episode:\n");
	epName = getString();
	epPtr = findEpisode(seasonPtr, epName);

	if (epPtr == NULL) {
		printf("Episode not found.\n");
		free(showName);
		free(seasonName);
		free(epName);
		return;
	}

	printf("Name: %s\n", epPtr->name);
	printf("Length: %s\n", epPtr->length);

	free(showName);
	free(seasonName);
	free(epName);
}

void printShow(void) {
	char* showName = NULL;
	TVShow* showPtr = NULL;

	Season* s = NULL;
	Episode* e = NULL;

	int sIdx = 0;
	int eIdx = 0;

	printf("Enter the name of the show:\n");
	showName = getString();
	showPtr = findShow(showName);

	if (showPtr == NULL) {
		printf("Show not found.\n");
		free(showName);
		return;
	}

	printf("Name: %s\n", showPtr->name);
	printf("Seasons:\n");

	s = showPtr->seasons;
	while (s != NULL) {
		printf("\tSeason %d: %s\n", sIdx, s->name);

		eIdx = 0;
		e = s->episodes;
		while (e != NULL) {
			printf("\t\tEpisode %d: %s (%s)\n", eIdx, e->name, e->length);
			eIdx++;
			e = e->next;
		}

		sIdx++;
		s = s->next;
	}

	free(showName);
}

void printArray(void) {
	int r = 0;
	int c = 0;

	if (database == NULL || dbSize == 0) {
		return;
	}

	for (r = 0; r < dbSize; r++) {
		for (c = 0; c < dbSize; c++) {
			if (database[r][c] != NULL) {
				printf("[%s] ", database[r][c]->name);
			}
			else {
				printf("[NULL] ");
			}
		}
		printf("\n");
	}
}

/* ---------- Menus (given-ish) ---------- */

void addMenu(void) {
	int choice = 0;

	printf("Choose an option:\n");
	printf("1. Add a TV show\n");
	printf("2. Add a season\n");
	printf("3. Add an episode\n");
	scanf("%d", &choice);
	getchar();

	switch (choice) {
	case 1:
		addShow();
		break;
	case 2:
		addSeason();
		break;
	case 3:
		addEpisode();
		break;
	default:
		break;
	}
}

void deleteMenu(void) {
	int choice = 0;

	printf("Choose an option:\n");
	printf("1. Delete a TV show\n");
	printf("2. Delete a season\n");
	printf("3. Delete an episode\n");
	scanf("%d", &choice);
	getchar();

	switch (choice) {
	case 1:
		deleteShow();
		break;
	case 2:
		deleteSeason();
		break;
	case 3:
		deleteEpisode();
		break;
	default:
		break;
	}
}

void printMenuSub(void) {
	int choice = 0;

	printf("Choose an option:\n");
	printf("1. Print a TV show\n");
	printf("2. Print an episode\n");
	printf("3. Print the array\n");
	scanf("%d", &choice);
	getchar();

	switch (choice) {
	case 1:
		printShow();
		break;
	case 2:
		printEpisode();
		break;
	case 3:
		printArray();
		break;
	default:
		break;
	}
}

void mainMenu(void) {
	printf("Choose an option:\n");
	printf("1. Add\n");
	printf("2. Delete\n");
	printf("3. Print\n");
	printf("4. Exit\n");
}

int main(void) {
	int choice = 0;

	do {
		mainMenu();
		scanf("%d", &choice);
		getchar();

		switch (choice) {
		case 1:
			addMenu();
			break;
		case 2:
			deleteMenu();
			break;
		case 3:
			printMenuSub();
			break;
		case 4:
			freeAll();
			break;
		default:
			break;
		}
	} while (choice != 4);

	return 0;
}
