//source code - turing machine simulator(TimSim)
//Author - Krishna Singh
//Phase 1 : Transition input | Phase 2 : Validation | Phase 3 : Execution
//Phase 4 : Command line options, machine files, built-in demo, update checker
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

// Linux / macOS / WSL use the POSIX calls, Windows (MinGW build) uses the underscore versions.
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#define tim_isatty _isatty
#define tim_popen _popen
#define tim_pclose _pclose
#define tim_mkdir(p) _mkdir(p)
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define URL_SAFE_CHARS ":/._-~?=@+,"      // & and % are special for cmd.exe, so they are not allowed
#else
#include <unistd.h>
#define tim_isatty isatty
#define tim_popen popen
#define tim_pclose pclose
#define tim_mkdir(p) mkdir(p, 0755)
#define URL_SAFE_CHARS ":/._-~?=&%@+,"
#endif

// ===================== Section-1 : Constants and data structures =====================
// TIMSIM_VERSION and TIMSIM_PREFIX are passed in by the Makefile (version comes from the VERSION file)
#ifndef TIMSIM_VERSION
#define TIMSIM_VERSION "0.0.0"
#endif
#ifndef TIMSIM_PREFIX
#define TIMSIM_PREFIX "/usr/local"
#endif
#define REPO_URL "https://github.com/krishnacode22/TimSim"
#define RAW_URL "https://raw.githubusercontent.com/krishnacode22/TimSim/main"
#define CHECK_INTERVAL 86400  // automatic update check happens at most once a day (seconds)

#define MAX_TRANSITIONS 30
#define MAX_STATE 99          // states are whole numbers from 0 to MAX_STATE
#define MAX_ACCEPT 10         // maximum number of accepting states
#define MAX_REJECT 10         // maximum number of rejecting states
#define MAX_INPUT 200         // maximum length of the input string
#define MAX_STEP_LIMIT 100000 // largest step limit the user can choose
#define DEFAULT_STEP_LIMIT 1000
#define BLANK '_'             // the blank symbol of the tape
#define TAPE_SIZE 20001       // fixed tape size (the head can never go past its ends, see Section-9)
#define TAPE_START 10000      // the head starts in the middle, so it has room on both sides
#define LINE_SIZE 256

// structure delta(transitions) = (qi,read_symbol,write_symbol,qf,direction_to_move)
typedef struct
{
	int from;
	int to;
	char read;
	char write;
	char move;

}delta;

int main(int argc, char *argv[])
{
	// ===================== Section-2 : Configuration =====================
	int n = 0; // number of transitions
	delta t[MAX_TRANSITIONS];

	int start_state = 0;
	int accept[MAX_ACCEPT];
	int reject[MAX_REJECT];
	int accept_count = 0;
	int reject_count = 0;
	int step_limit = DEFAULT_STEP_LIMIT;

	char line[LINE_SIZE]; // every line typed by the user is read into this buffer
	size_t len;
	int ch;               // used to throw away the rest of a too long line
	long val;             // numbers are read as long first so overflow can not sneak in
	long val2;
	int used;             // how many characters sscanf consumed (%n)

	// command line settings
	const char *file_path = NULL; // machine file given on the command line
	const char *cli_input = NULL; // input string given with --input
	int show_help = 0;
	int show_version = 0;
	int cli_trace = 0;
	int quiet = 0;
	int check_only = 0;
	int demo = 0;
	int no_update = 0;
	int upd_mode = 0;             // 0 = automatic check, 2 = --check-update, 3 = --update
	long cli_limit = 0;

	// ===================== Section-3 : Command line options =====================
	for (int a = 1; a < argc; a++)
	{
		const char *arg = argv[a];
		int is_input = (strcmp(arg, "-i") == 0 || strcmp(arg, "--input") == 0);
		int is_limit = (strcmp(arg, "-l") == 0 || strcmp(arg, "--limit") == 0);
		int is_run = (strcmp(arg, "--run") == 0);

		if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) show_help = 1;
		else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) show_version = 1;
		else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--trace") == 0) cli_trace = 1;
		else if (strcmp(arg, "-q") == 0 || strcmp(arg, "--quiet") == 0) quiet = 1;
		else if (strcmp(arg, "--check") == 0) check_only = 1;
		else if (strcmp(arg, "--demo") == 0) demo = 1;
		else if (strcmp(arg, "--check-update") == 0) upd_mode = 2;
		else if (strcmp(arg, "--update") == 0) upd_mode = 3;
		else if (strcmp(arg, "--no-update-check") == 0) no_update = 1;
		else if (is_input || is_limit || is_run)
		{
			if (a + 1 >= argc)
			{
				fprintf(stderr, "timsim: option '%s' needs a value.\nTry 'timsim --help'.\n", arg);
				return 2;
			}
			a++;
			if (is_input)
				cli_input = argv[a];
			else if (is_limit)
			{
				used = -1;
				if (sscanf(argv[a], " %ld %n", &val, &used) != 1 || argv[a][used] != '\0'
					|| val < 1 || val > MAX_STEP_LIMIT)
				{
					fprintf(stderr, "timsim: step limit must be a whole number from 1 to %d.\n", MAX_STEP_LIMIT);
					return 2;
				}
				cli_limit = val;
			}
			else
			{
				if (file_path != NULL)
				{
					fprintf(stderr, "timsim: only one machine file can be given.\n");
					return 2;
				}
				file_path = argv[a];
			}
		}
		else if (arg[0] == '-' && arg[1] != '\0')
		{
			fprintf(stderr, "timsim: unknown option '%s'.\nTry 'timsim --help'.\n", arg);
			return 2;
		}
		else
		{
			if (file_path != NULL)
			{
				fprintf(stderr, "timsim: only one machine file can be given.\n");
				return 2;
			}
			file_path = arg;
		}
	}

	if (show_help)
	{
		printf("TimSim %s - Turing machine simulator\n\n", TIMSIM_VERSION);
		printf("Usage:\n");
		printf("  timsim                       interactive mode (type the machine in)\n");
		printf("  timsim FILE                  load a machine file, then ask for the input string\n");
		printf("  timsim FILE -i STRING        run FILE on STRING and exit (good for scripts)\n");
		printf("  timsim --demo                run a built-in example machine with a trace\n\n");
		printf("Options:\n");
		printf("  -i, --input STRING   input string to run (no prompts; needs FILE or --demo)\n");
		printf("  -t, --trace          show the step-by-step trace\n");
		printf("  -l, --limit N        step limit, 1 to %d (overrides the machine file)\n", MAX_STEP_LIMIT);
		printf("  -q, --quiet          with --input: print only the result and the final tape\n");
		printf("      --run FILE       same as giving FILE directly\n");
		printf("      --check          only validate FILE, do not run it\n");
		printf("      --demo           run the built-in binary incrementer\n");
		printf("      --check-update   check GitHub for a newer version\n");
		printf("      --update         install the newest version from GitHub\n");
		printf("      --no-update-check  skip the automatic daily update check\n");
		printf("  -v, --version        show the version\n");
		printf("  -h, --help           show this help\n\n");
		printf("Exit status with --input:  0 accepted, 1 rejected, 2 error, 3 no decision\n");
		printf("                           (step limit or tape boundary reached)\n\n");
		printf("Machine file format (one item per line, text after # is a comment):\n");
		printf("  start 0                 start state\n");
		printf("  accept 2                accepting state(s), separated by spaces\n");
		printf("  reject 3                rejecting state(s), optional\n");
		printf("  limit 1000              step limit, optional\n");
		printf("  0 1 a b R               transition: from to read write move(L/R/S)\n\n");
		printf("Environment: TIMSIM_NO_UPDATE_CHECK=1 turns the automatic update check off.\n");
		printf("More information: %s\n", REPO_URL);
		return 0;
	}
	if (show_version)
	{
		printf("timsim %s\n", TIMSIM_VERSION);
		return 0;
	}
	if (demo && file_path != NULL)
	{
		fprintf(stderr, "timsim: use either --demo or a machine file, not both.\n");
		return 2;
	}
	if (check_only && file_path == NULL)
	{
		fprintf(stderr, "timsim: --check needs a machine file.\nTry 'timsim --help'.\n");
		return 2;
	}
	if (cli_input != NULL && file_path == NULL && !demo)
	{
		fprintf(stderr, "timsim: --input needs a machine file (or --demo).\nTry 'timsim --help'.\n");
		return 2;
	}

	// ===================== Section-4 : Update checker =====================
	// Asks GitHub for the VERSION file (using curl or wget, whichever exists).
	// Automatic check: only in a real terminal, at most once a day, never in scripts.
	// Nothing is ever installed without the user saying yes.
	char cache_dir[512] = "";   // where the "last checked" note is kept
	char cache_file[600] = "";
	char skipped[32] = "-";     // version the user chose to skip
	long last_check = 0;
	time_t now = time(NULL);
	int do_check = upd_mode;
	if (do_check == 0 && !no_update && getenv("TIMSIM_NO_UPDATE_CHECK") == NULL
		&& cli_input == NULL && !check_only && !demo
		&& tim_isatty(STDIN_FILENO) && tim_isatty(STDOUT_FILENO))
		do_check = 1;

	if (do_check)
	{
		const char *xdg = getenv("XDG_CACHE_HOME");
		const char *home = getenv("HOME");
#ifdef _WIN32
		if (xdg == NULL || xdg[0] == '\0') xdg = getenv("LOCALAPPDATA"); // C:\Users\name\AppData\Local
#endif

		if (xdg != NULL && xdg[0] != '\0')
			snprintf(cache_dir, sizeof(cache_dir), "%s/timsim", xdg);
		else if (home != NULL && home[0] != '\0')
			snprintf(cache_dir, sizeof(cache_dir), "%s/.cache/timsim", home);
		if (cache_dir[0] != '\0')
			snprintf(cache_file, sizeof(cache_file), "%s/update_check", cache_dir);

		// cache file = "<time of last check> <version the user chose to skip>"
		if (cache_file[0] != '\0')
		{
			FILE *cf = fopen(cache_file, "r");
			if (cf != NULL)
			{
				if (fscanf(cf, "%ld %31s", &last_check, skipped) < 1) last_check = 0;
				fclose(cf);
			}
		}

		// automatic check: no cache location, or checked less than a day ago -> do nothing
		if (upd_mode == 0 && (cache_file[0] == '\0'
			|| (last_check > 0 && now >= last_check && now - last_check < CHECK_INTERVAL)))
			do_check = 0;
	}

	if (do_check)
	{
		const char *url = getenv("TIMSIM_VERSION_URL"); // can be changed for testing
		char cmd[1024];
		char remote[64] = "";
		int url_ok = 1;
		int fetched = 0;
		int valid = 0;
		int rmaj = 0, rmin = 0, rpat = 0;
		int lmaj = 0, lmin = 0, lpat = 0;
		int newer = 0;

		if (url == NULL || url[0] == '\0') url = RAW_URL "/VERSION";
		for (const char *c = url; *c != '\0'; c++)
			if (!isalnum((unsigned char)*c) && strchr(URL_SAFE_CHARS, *c) == NULL) url_ok = 0;

		if (url_ok)
		{
			FILE *p;
#ifdef _WIN32
			// Windows 10 and later ship curl.exe
			snprintf(cmd, sizeof(cmd), "curl -fsS --max-time 3 \"%s\" 2>nul", url);
#else
			snprintf(cmd, sizeof(cmd), "(curl -fsS --max-time 3 '%s' || wget -qO- -T 3 '%s') 2>/dev/null", url, url);
#endif
			p = tim_popen(cmd, "r");
			if (p != NULL)
			{
				if (fgets(remote, sizeof(remote), p) != NULL) fetched = 1;
				tim_pclose(p);
			}
		}

		// the downloaded text is checked strictly (digits and dots only) before it is trusted
		if (fetched)
		{
			remote[strcspn(remote, " \t\r\n")] = '\0';
			used = -1;
			if (strlen(remote) >= 5 && strlen(remote) <= 15
				&& strspn(remote, "0123456789.") == strlen(remote)
				&& sscanf(remote, "%d.%d.%d%n", &rmaj, &rmin, &rpat, &used) == 3
				&& remote[used] == '\0')
				valid = 1;
		}
		sscanf(TIMSIM_VERSION, "%d.%d.%d", &lmaj, &lmin, &lpat);
		newer = valid && (rmaj > lmaj || (rmaj == lmaj && (rmin > lmin || (rmin == lmin && rpat > lpat))));

		// remember that we checked (so the automatic check runs once a day)
		if (cache_file[0] != '\0')
		{
			char parent[512];
			char *slash;
			FILE *cf;
			snprintf(parent, sizeof(parent), "%s", cache_dir);
			slash = strrchr(parent, '/');
			if (slash != NULL) { *slash = '\0'; tim_mkdir(parent); }
			tim_mkdir(cache_dir);
			cf = fopen(cache_file, "w");
			if (cf != NULL)
			{
				fprintf(cf, "%ld %s\n", (long)now, skipped);
				fclose(cf);
			}
		}

		if (!valid)
		{
			if (upd_mode)
			{
				fprintf(stderr, "Could not check for updates (no internet connection, or neither curl nor wget is installed).\n");
				return 1;
			}
		}
		else if (!newer)
		{
			if (upd_mode)
			{
				printf("TimSim is up to date (version %s).\n", TIMSIM_VERSION);
				return 0;
			}
		}
		else if (upd_mode == 0 && strcmp(skipped, remote) == 0)
		{
			// the user already chose to skip this version
		}
		else
		{
			int do_install = 0;
			printf("\n*** A new version of TimSim is available: %s -> %s ***\n", TIMSIM_VERSION, remote);
			printf("    Release page: %s\n", REPO_URL);

			if (upd_mode == 3)
				do_install = 1;
			else if (upd_mode == 2)
			{
				printf("Run 'timsim --update' to install it.\n");
				return 4;
			}
			else
			{
				while (1)
				{
					char c;
					printf("Update now? [y]es / [n]o / [s]kip this version: ");
					if (fgets(line, sizeof(line), stdin) == NULL)
					{
						printf("\n");
						break;
					}
					len = strlen(line);
					if (len == sizeof(line) - 1 && line[len - 1] != '\n')
						while ((ch = getchar()) != '\n' && ch != EOF);
					c = (char)tolower((unsigned char)line[strspn(line, " \t")]);
					if (c == 'y') { do_install = 1; break; }
					if (c == 'n' || c == '\n' || c == '\0') break;
					if (c == 's')
					{
						if (cache_file[0] != '\0')
						{
							FILE *cf = fopen(cache_file, "w");
							if (cf != NULL)
							{
								fprintf(cf, "%ld %s\n", (long)now, remote);
								fclose(cf);
							}
						}
						printf("OK, TimSim will not remind you about version %s again.\n", remote);
						break;
					}
					printf("Please answer y, n or s.\n");
				}
			}

			if (do_install)
			{
				char icmd[1400];
				const char *iu = getenv("TIMSIM_INSTALL_URL"); // can be changed for testing
				int iu_ok = 1;
#ifdef _WIN32
				if (iu == NULL || iu[0] == '\0') iu = RAW_URL "/install.ps1";
#else
				if (iu == NULL || iu[0] == '\0') iu = RAW_URL "/install.sh";
#endif
				for (const char *c = iu; *c != '\0'; c++)
					if (!isalnum((unsigned char)*c) && strchr(URL_SAFE_CHARS, *c) == NULL) iu_ok = 0;
				if (!iu_ok)
				{
					fprintf(stderr, "timsim: TIMSIM_INSTALL_URL contains unsafe characters.\n");
					return 1;
				}
#ifdef _WIN32
				// PowerShell downloads and runs install.ps1; any failure stops it with a non-zero exit code
				snprintf(icmd, sizeof(icmd),
					"powershell -NoProfile -ExecutionPolicy Bypass -Command "
					"\"$ErrorActionPreference='Stop'; iex (irm '%s')\"", iu);
#else
				// the installer is downloaded first, so a failed download can not look like a success
				snprintf(icmd, sizeof(icmd),
					"script=$(curl -fsSL '%s' 2>/dev/null || wget -qO- '%s' 2>/dev/null); "
					"if [ -z \"$script\" ]; then echo 'Error: could not download the installer.' >&2; exit 1; fi; "
					"PREFIX='%s' sh -c \"$script\"", iu, iu, TIMSIM_PREFIX);
#endif
				printf("\nRunning the official installer from GitHub...\n");
				if (system(icmd) == 0)
				{
					printf("\nUpdate finished. Run 'timsim' again to use version %s.\n", remote);
					return 0;
				}
				printf("\nUpdate failed. You can try again later with 'timsim --update'.\n");
				if (upd_mode) return 1;
			}
			printf("\n");
		}
	}

	// ===================== Section-5 : Getting the machine =====================
	// Three ways to get a machine: --demo (built in), a machine file, or typing it in.
	if (demo)
	{
		static const delta demo_table[6] = {
			{0, 0, '0', '0', 'R'},
			{0, 0, '1', '1', 'R'},
			{0, 1, '_', '_', 'L'},
			{1, 1, '1', '0', 'L'},
			{1, 2, '0', '1', 'S'},
			{1, 2, '_', '1', 'S'}
		};
		memcpy(t, demo_table, sizeof(demo_table));
		n = 6;
		start_state = 0;
		accept[0] = 2;
		accept_count = 1;
		if (cli_input == NULL) cli_input = "1011";
		if (!quiet) cli_trace = 1;
		if (!quiet)
		{
			printf("TimSim demo: binary incrementer (adds 1 to a binary number)\n");
			printf("State 0 runs right to the end of the number, state 1 walks back\n");
			printf("turning 1s into 0s until it can write a 1. State 2 accepts.\n");
			printf("Try your own: timsim --demo -i 1111\n\n");
		}
	}
	else if (file_path != NULL)
	{
		FILE *mf = fopen(file_path, "r");
		int lineno = 0;
		int file_errors = 0;
		int tline[MAX_TRANSITIONS]; // file line of each transition, used in conflict messages
		int start_set = 0, accept_set = 0, reject_set = 0, limit_set = 0;

		if (mf == NULL)
		{
			fprintf(stderr, "timsim: cannot open '%s': %s\n", file_path, strerror(errno));
			return 2;
		}

		while (fgets(line, sizeof(line), mf) != NULL)
		{
			char *p;
			lineno++;
			len = strlen(line);
			if (len == sizeof(line) - 1 && line[len - 1] != '\n')
			{
				while ((ch = fgetc(mf)) != '\n' && ch != EOF);
				fprintf(stderr, "%s:%d: line too long\n", file_path, lineno);
				file_errors++;
				continue;
			}
			line[strcspn(line, "\r\n")] = '\0';
			p = line + strspn(line, " \t");
			if (*p == '\0' || *p == '#') continue; // empty line or comment

			if (isalpha((unsigned char)*p))
			{
				// ---- directive: start / accept / reject / limit ----
				char kw[16];
				char *rest;
				used = -1;
				if (sscanf(p, "%15s %n", kw, &used) != 1 || used < 0)
				{
					fprintf(stderr, "%s:%d: cannot read this line\n", file_path, lineno);
					file_errors++;
					continue;
				}
				rest = p + used;
				if (strchr(rest, '#') != NULL) *strchr(rest, '#') = '\0'; // text after # is a comment

				if (strcmp(kw, "start") == 0 || strcmp(kw, "limit") == 0)
				{
					used = -1;
					if (sscanf(rest, "%ld %n", &val, &used) != 1 || rest[used] != '\0')
					{
						fprintf(stderr, "%s:%d: '%s' needs exactly one whole number\n", file_path, lineno, kw);
						file_errors++;
						continue;
					}
					if (kw[0] == 's')
					{
						if (start_set) { fprintf(stderr, "%s:%d: 'start' is defined twice\n", file_path, lineno); file_errors++; continue; }
						if (val < 0 || val > MAX_STATE) { fprintf(stderr, "%s:%d: state must be 0 to %d\n", file_path, lineno, MAX_STATE); file_errors++; continue; }
						start_state = (int)val;
						start_set = 1;
					}
					else
					{
						if (limit_set) { fprintf(stderr, "%s:%d: 'limit' is defined twice\n", file_path, lineno); file_errors++; continue; }
						if (val < 1 || val > MAX_STEP_LIMIT) { fprintf(stderr, "%s:%d: limit must be 1 to %d\n", file_path, lineno, MAX_STEP_LIMIT); file_errors++; continue; }
						step_limit = (int)val;
						limit_set = 1;
					}
				}
				else if (strcmp(kw, "accept") == 0 || strcmp(kw, "reject") == 0)
				{
					int is_accept = (kw[0] == 'a');
					int *list = is_accept ? accept : reject;
					int *count = is_accept ? &accept_count : &reject_count;
					int max = is_accept ? MAX_ACCEPT : MAX_REJECT;
					int *already = is_accept ? &accept_set : &reject_set;
					int pos = 0;
					int bad = 0;
					char junk;

					if (*already) { fprintf(stderr, "%s:%d: '%s' is defined twice\n", file_path, lineno, kw); file_errors++; continue; }
					*already = 1;
					*count = 0;
					while (sscanf(rest + pos, " %ld%n", &val, &used) == 1)
					{
						pos += used;
						if (val < 0 || val > MAX_STATE) { bad = 1; break; }
						if (*count >= max) { bad = 2; break; }
						for (int k = 0; k < *count; k++)
							if (list[k] == (int)val) bad = 3;
						if (bad) break;
						list[(*count)++] = (int)val;
					}
					if (!bad && sscanf(rest + pos, " %c", &junk) == 1) bad = 4;
					if (!bad && is_accept && *count == 0) bad = 5;

					if (bad == 1) fprintf(stderr, "%s:%d: states must be 0 to %d\n", file_path, lineno, MAX_STATE);
					else if (bad == 2) fprintf(stderr, "%s:%d: too many states (max %d)\n", file_path, lineno, max);
					else if (bad == 3) fprintf(stderr, "%s:%d: a state is listed twice\n", file_path, lineno);
					else if (bad == 4) fprintf(stderr, "%s:%d: '%s' takes state numbers only\n", file_path, lineno, kw);
					else if (bad == 5) fprintf(stderr, "%s:%d: 'accept' needs at least one state\n", file_path, lineno);
					if (bad) { file_errors++; continue; }
				}
				else
				{
					fprintf(stderr, "%s:%d: unknown word '%s' (use start, accept, reject or limit)\n", file_path, lineno, kw);
					file_errors++;
					continue;
				}
			}
			else
			{
				// ---- transition: from to read write move ----
				char rs[8], ws[8], ms[8];
				int conflict_with = 0;
				used = -1;
				if (sscanf(p, " %ld %ld %7s %7s %7s %n", &val, &val2, rs, ws, ms, &used) != 5 || (p[used] != '\0' && p[used] != '#'))
				{
					fprintf(stderr, "%s:%d: expected 5 values: from to read write move\n", file_path, lineno);
					file_errors++;
					continue;
				}
				if (val < 0 || val > MAX_STATE || val2 < 0 || val2 > MAX_STATE)
				{
					fprintf(stderr, "%s:%d: states must be 0 to %d\n", file_path, lineno, MAX_STATE);
					file_errors++;
					continue;
				}
				if (strlen(rs) != 1 || strlen(ws) != 1 || strlen(ms) != 1
					|| !isgraph((unsigned char)rs[0]) || !isgraph((unsigned char)ws[0]))
				{
					fprintf(stderr, "%s:%d: symbols and move must be one visible character each\n", file_path, lineno);
					file_errors++;
					continue;
				}
				if (ms[0] >= 'a' && ms[0] <= 'z')
					ms[0] -= 'a' - 'A';
				if (ms[0] != 'L' && ms[0] != 'R' && ms[0] != 'S')
				{
					fprintf(stderr, "%s:%d: invalid direction '%c' (use L, R or S)\n", file_path, lineno, ms[0]);
					file_errors++;
					continue;
				}
				for (int j = 0; j < n; j++)
					if (t[j].from == (int)val && t[j].read == rs[0]) { conflict_with = tline[j]; break; }
				if (conflict_with)
				{
					fprintf(stderr, "%s:%d: conflict - line %d already handles state %ld reading '%c'\n",
							file_path, lineno, conflict_with, val, rs[0]);
					file_errors++;
					continue;
				}
				if (n >= MAX_TRANSITIONS)
				{
					fprintf(stderr, "%s:%d: too many transitions (max %d)\n", file_path, lineno, MAX_TRANSITIONS);
					file_errors++;
					continue;
				}
				t[n].from = (int)val;
				t[n].to = (int)val2;
				t[n].read = rs[0];
				t[n].write = ws[0];
				t[n].move = ms[0];
				tline[n] = lineno;
				n++;
			}
		}
		fclose(mf);

		if (!start_set) { fprintf(stderr, "%s: missing 'start' line\n", file_path); file_errors++; }
		if (!accept_set) { fprintf(stderr, "%s: missing 'accept' line\n", file_path); file_errors++; }
		if (n == 0) { fprintf(stderr, "%s: no transitions found\n", file_path); file_errors++; }
		for (int i = 0; i < accept_count; i++)
			for (int j = 0; j < reject_count; j++)
				if (accept[i] == reject[j])
				{
					fprintf(stderr, "%s: state %d is both accepting and rejecting\n", file_path, accept[i]);
					file_errors++;
				}

		if (file_errors > 0)
		{
			fprintf(stderr, "%s: %d error(s) found, machine not loaded.\n", file_path, file_errors);
			return 2;
		}
		if (!quiet)
			printf("Loaded '%s': %d transitions.\n", file_path, n);
	}
	else
	{
		printf("========================================\n");
		printf("   TimSim v%s - Turing Machine Simulator\n", TIMSIM_VERSION);
		printf("========================================\n");
		printf("Rules used by this simulator:\n");
		printf(" - States are whole numbers from 0 to %d\n", MAX_STATE);
		printf(" - Tape symbols are any visible character (no spaces), blank symbol is '%c'\n", BLANK);
		printf(" - The machine is deterministic (one transition per state + read symbol)\n");
		printf(" - Halts when it reaches an accepting or rejecting state, or when no\n");
		printf("   transition exists for the current state and symbol (counts as reject)\n");
		printf("Tip: run 'timsim --help' to see the command line options.\n");
		printf("========================================\n\n");

		// ===================== Section-3 : Transition input =====================
		while (1)
		{
			printf("Enter Number of Transitions (1 to %d) : ", MAX_TRANSITIONS);
			if (fgets(line, sizeof(line), stdin) == NULL)
			{
				printf("\nInput ended unexpectedly.\n");
				return 1;
			}
			len = strlen(line);
			if (len == sizeof(line) - 1 && line[len - 1] != '\n')
			{
				while ((ch = getchar()) != '\n' && ch != EOF);
				printf("Line too long. Try again.\n");
				continue;
			}
			used = -1;
			if (sscanf(line, " %ld %n", &val, &used) != 1 || line[used] != '\0')
			{
				printf("Invalid input. Please enter a whole number.\n");
				continue;
			}
			if (val < 1 || val > MAX_TRANSITIONS)
			{
				printf("Number of transitions must be between 1 and %d.\n", MAX_TRANSITIONS);
				continue;
			}
			n = (int)val;
			break;
		}

		printf("\n========================================\n");
		printf("Transition Format, seperate by space when entering \n{Current State} {Next State} {Read symbol} {Write symbol} {Move}\n--------------------------------------------------------------------------------------\n");
		printf("Example : 0 1 a b R   (in state 0, read 'a', write 'b', go to state 1, move right)\n");
		printf("Allowed moving directions = {l,r,s} or {L,R,S} Left, Right and Stay respectively\n");
		printf("Blank symbol is '%c'. States are 0 to %d.\n========================================\n", BLANK, MAX_STATE);

		for (int i = 0; i < n; i++)
		{
			while (1)
			{
				char rs[8], ws[8], ms[8]; // symbols are read as words so "ab" can be detected
				printf("Enter Transition number %d: ", i + 1);
				if (fgets(line, sizeof(line), stdin) == NULL)
				{
					printf("\nInput ended unexpectedly.\n");
					return 1;
				}
				len = strlen(line);
				if (len == sizeof(line) - 1 && line[len - 1] != '\n')
				{
					while ((ch = getchar()) != '\n' && ch != EOF);
					printf("Line too long. Try again.\n");
					continue;
				}

				// ===================== Section-4 : Input validation =====================
				used = -1;
				if (sscanf(line, " %ld %ld %7s %7s %7s %n", &val, &val2, rs, ws, ms, &used) != 5
					|| line[used] != '\0')
				{
					printf("Invalid input. Expected exactly 5 values: {from} {to} {read} {write} {move}\n");
					continue;
				}
				if (val < 0 || val > MAX_STATE || val2 < 0 || val2 > MAX_STATE)
				{
					printf("Invalid state! States must be whole numbers from 0 to %d.\n", MAX_STATE);
					continue;
				}
				if (strlen(rs) != 1 || strlen(ws) != 1 || strlen(ms) != 1)
				{
					printf("Invalid input! Read symbol, write symbol and move must be a single character each.\n");
					continue;
				}
				if (!isgraph((unsigned char)rs[0]) || !isgraph((unsigned char)ws[0]))
				{
					printf("Invalid symbol! Use visible characters only (blank is '%c').\n", BLANK);
					continue;
				}

				//Validating move input
				if (ms[0] >= 'a' && ms[0] <= 'z')
					ms[0] -= 'a' - 'A';

				if (ms[0] != 'L' &&
					ms[0] != 'R' &&
					ms[0] != 'S')
				{
					printf("Invalid direction! Use L, R, or S.\n");
					continue;
				}

				// ===================== Section-5 : Conflict detection =====================
				// checked right when the transition is typed, so only this line has to be re-entered
				int conflict_with = 0;
				for (int j = 0; j < i; j++)
				{
					if (t[j].from == (int)val && t[j].read == rs[0])
					{
						conflict_with = j + 1;
						break;
					}
				}
				if (conflict_with)
				{
					printf("Conflict detected! Transition %d already handles state %ld reading '%c'.\n",
							conflict_with, val, rs[0]);
					printf("A deterministic machine allows only one transition per (state, read symbol). Try again.\n");
					continue;
				}

				t[i].from = (int)val;
				t[i].to = (int)val2;
				t[i].read = rs[0];
				t[i].write = ws[0];
				t[i].move = ms[0];
				break;  //Valid transition; exit while loop
			}
		}
		printf("Input completed\n");

		//Displaying stored input
		printf("\nEntered Transition Table\n");
		printf("-------------------------------\n");
		printf(" No | From | To | Read | Write | Move\n");
		printf("-------------------------------\n");
		for (int i = 0; i < n; i++)
		{
			printf("%3d | %4d | %2d |  %c   |   %c   |  %c\n", i + 1,
					t[i].from,
					t[i].to,
					t[i].read,
					t[i].write,
					t[i].move);
		}
		printf("-------------------------------\n");
		printf("Transition validation passed.\n");

		// ===================== Section-2 (continued) : Machine configuration =====================
		printf("\n========================================\n");
		printf("Machine configuration\n");
		printf("========================================\n");

		// start state
		while (1)
		{
			printf("Enter start state (0 to %d): ", MAX_STATE);
			if (fgets(line, sizeof(line), stdin) == NULL)
			{
				printf("\nInput ended unexpectedly.\n");
				return 1;
			}
			len = strlen(line);
			if (len == sizeof(line) - 1 && line[len - 1] != '\n')
			{
				while ((ch = getchar()) != '\n' && ch != EOF);
				printf("Line too long. Try again.\n");
				continue;
			}
			used = -1;
			if (sscanf(line, " %ld %n", &val, &used) != 1 || line[used] != '\0')
			{
				printf("Invalid input. Please enter one whole number.\n");
				continue;
			}
			if (val < 0 || val > MAX_STATE)
			{
				printf("Invalid state! States must be whole numbers from 0 to %d.\n", MAX_STATE);
				continue;
			}
			start_state = (int)val;
			break;
		}

		// accepting states (at least one)
		while (1)
		{
			int bad = 0;
			int pos = 0;
			char junk;
			printf("Enter accepting state(s), separated by spaces (max %d): ", MAX_ACCEPT);
			if (fgets(line, sizeof(line), stdin) == NULL)
			{
				printf("\nInput ended unexpectedly.\n");
				return 1;
			}
			len = strlen(line);
			if (len == sizeof(line) - 1 && line[len - 1] != '\n')
			{
				while ((ch = getchar()) != '\n' && ch != EOF);
				printf("Line too long. Try again.\n");
				continue;
			}
			accept_count = 0;
			while (sscanf(line + pos, " %ld%n", &val, &used) == 1)
			{
				pos += used;
				if (val < 0 || val > MAX_STATE) { bad = 1; break; }
				if (accept_count >= MAX_ACCEPT) { bad = 2; break; }
				for (int k = 0; k < accept_count; k++)
					if (accept[k] == (int)val) bad = 3;
				if (bad) break;
				accept[accept_count++] = (int)val;
			}
			if (!bad && sscanf(line + pos, " %c", &junk) == 1) bad = 4;
			if (!bad && accept_count == 0) bad = 5;

			if (bad == 1) printf("Invalid state! States must be whole numbers from 0 to %d.\n", MAX_STATE);
			else if (bad == 2) printf("Too many accepting states (max %d).\n", MAX_ACCEPT);
			else if (bad == 3) printf("The same state was listed twice.\n");
			else if (bad == 4) printf("Invalid input. Enter state numbers only, separated by spaces.\n");
			else if (bad == 5) printf("At least one accepting state is required, otherwise the machine can never accept.\n");
			if (bad) continue;
			break;
		}

		// rejecting states (optional, may be empty)
		while (1)
		{
			int bad = 0;
			int pos = 0;
			char junk;
			printf("Enter rejecting state(s), separated by spaces (press Enter for none): ");
			if (fgets(line, sizeof(line), stdin) == NULL)
			{
				printf("\nInput ended unexpectedly.\n");
				return 1;
			}
			len = strlen(line);
			if (len == sizeof(line) - 1 && line[len - 1] != '\n')
			{
				while ((ch = getchar()) != '\n' && ch != EOF);
				printf("Line too long. Try again.\n");
				continue;
			}
			reject_count = 0;
			while (sscanf(line + pos, " %ld%n", &val, &used) == 1)
			{
				pos += used;
				if (val < 0 || val > MAX_STATE) { bad = 1; break; }
				if (reject_count >= MAX_REJECT) { bad = 2; break; }
				for (int k = 0; k < reject_count; k++)
					if (reject[k] == (int)val) bad = 3;
				for (int k = 0; k < accept_count; k++)
					if (accept[k] == (int)val) bad = 6;
				if (bad) break;
				reject[reject_count++] = (int)val;
			}
			if (!bad && sscanf(line + pos, " %c", &junk) == 1) bad = 4;

			if (bad == 1) printf("Invalid state! States must be whole numbers from 0 to %d.\n", MAX_STATE);
			else if (bad == 2) printf("Too many rejecting states (max %d).\n", MAX_REJECT);
			else if (bad == 3) printf("The same state was listed twice.\n");
			else if (bad == 4) printf("Invalid input. Enter state numbers only, separated by spaces.\n");
			else if (bad == 6) printf("State %ld is already an accepting state. A state can not be both.\n", val);
			if (bad) continue;
			break;
		}

		// step limit (optional, skipped when --limit was given)
		while (cli_limit == 0)
		{
			printf("Enter step limit (1 to %d, press Enter for %d): ", MAX_STEP_LIMIT, DEFAULT_STEP_LIMIT);
			if (fgets(line, sizeof(line), stdin) == NULL)
			{
				printf("\nInput ended unexpectedly.\n");
				return 1;
			}
			len = strlen(line);
			if (len == sizeof(line) - 1 && line[len - 1] != '\n')
			{
				while ((ch = getchar()) != '\n' && ch != EOF);
				printf("Line too long. Try again.\n");
				continue;
			}
			if (line[strspn(line, " \t\r\n")] == '\0')
			{
				step_limit = DEFAULT_STEP_LIMIT;
				break;
			}
			used = -1;
			if (sscanf(line, " %ld %n", &val, &used) != 1 || line[used] != '\0')
			{
				printf("Invalid input. Please enter a whole number.\n");
				continue;
			}
			if (val < 1 || val > MAX_STEP_LIMIT)
			{
				printf("Step limit must be between 1 and %d.\n", MAX_STEP_LIMIT);
				continue;
			}
			step_limit = (int)val;
			break;
		}

		// offer to save the machine so it can be loaded again later (timsim FILE)
		while (1)
		{
			char fname[LINE_SIZE];
			char *name;
			FILE *sf;
			printf("\nSave this machine to a file? Enter a file name (or press Enter to skip): ");
			if (fgets(line, sizeof(line), stdin) == NULL)
			{
				printf("\n");
				break;
			}
			len = strlen(line);
			if (len == sizeof(line) - 1 && line[len - 1] != '\n')
			{
				while ((ch = getchar()) != '\n' && ch != EOF);
				printf("File name too long. Try again.\n");
				continue;
			}
			line[strcspn(line, "\r\n")] = '\0';
			name = line + strspn(line, " \t");
			len = strlen(name);
			while (len > 0 && isspace((unsigned char)name[len - 1])) name[--len] = '\0';
			if (len == 0) break;
			strcpy(fname, name);

			sf = fopen(fname, "r");
			if (sf != NULL)
			{
				fclose(sf);
				printf("'%s' already exists. Overwrite? (y/n): ", fname);
				if (fgets(line, sizeof(line), stdin) == NULL) break;
				if (tolower((unsigned char)line[strspn(line, " \t")]) != 'y') continue;
			}
			sf = fopen(fname, "w");
			if (sf == NULL)
			{
				printf("Could not write '%s': %s\n", fname, strerror(errno));
				continue;
			}
			fprintf(sf, "# TimSim machine file (run it with: timsim %s)\n", fname);
			fprintf(sf, "start %d\n", start_state);
			fprintf(sf, "accept");
			for (int k = 0; k < accept_count; k++) fprintf(sf, " %d", accept[k]);
			fprintf(sf, "\n");
			if (reject_count > 0)
			{
				fprintf(sf, "reject");
				for (int k = 0; k < reject_count; k++) fprintf(sf, " %d", reject[k]);
				fprintf(sf, "\n");
			}
			fprintf(sf, "limit %d\n\n", step_limit);
			for (int i = 0; i < n; i++)
				fprintf(sf, "%d %d %c %c %c\n", t[i].from, t[i].to, t[i].read, t[i].write, t[i].move);
			fclose(sf);
			printf("Saved. Load it any time with:  timsim '%s'\n", fname);
			break;
		}
	}

	// --limit always wins over the file and over the prompt
	if (cli_limit > 0) step_limit = (int)cli_limit;

	// ===================== Section-6 : State validation and warnings =====================
	// Hard errors were already rejected above (range, duplicates, accept/reject overlap).
	// What is left can not be proven wrong, so these are warnings only.
	int warnings = 0;

	int start_has_transition = 0;
	int start_is_halting = 0;
	for (int i = 0; i < n; i++)
		if (t[i].from == start_state) start_has_transition = 1;
	for (int k = 0; k < accept_count; k++)
		if (accept[k] == start_state) start_is_halting = 1;
	for (int k = 0; k < reject_count; k++)
		if (reject[k] == start_state) start_is_halting = 1;
	if (!start_has_transition && !start_is_halting)
	{
		fprintf(stderr, "Warning: start state %d has no outgoing transition, the machine will halt immediately.\n", start_state);
		warnings++;
	}

	for (int k = 0; k < accept_count; k++)
	{
		int reachable = (accept[k] == start_state);
		for (int i = 0; i < n; i++)
			if (t[i].to == accept[k]) reachable = 1;
		if (!reachable)
		{
			fprintf(stderr, "Warning: accepting state %d is never used as a next state, it can never be reached.\n", accept[k]);
			warnings++;
		}
	}

	for (int i = 0; i < n; i++)
	{
		int halting_from = 0;
		for (int k = 0; k < accept_count; k++)
			if (accept[k] == t[i].from) halting_from = 1;
		for (int k = 0; k < reject_count; k++)
			if (reject[k] == t[i].from) halting_from = 1;
		if (halting_from)
		{
			fprintf(stderr, "Warning: transition %d starts from halting state %d, it will never be used.\n", i + 1, t[i].from);
			warnings++;
		}
	}

	if (check_only)
	{
		printf("OK: '%s' is a valid deterministic machine (%d transitions, %d warning(s)).\n", file_path, n, warnings);
		return 0;
	}

	if (!quiet)
	{
		printf("\nState validation passed with %d warning(s).\n", warnings);
		printf("Start state: %d | Accepting: ", start_state);
		for (int k = 0; k < accept_count; k++) printf("%d ", accept[k]);
		printf("| Rejecting: ");
		if (reject_count == 0) printf("none (a missing transition rejects) ");
		for (int k = 0; k < reject_count; k++) printf("%d ", reject[k]);
		printf("| Step limit: %d\n", step_limit);
	}

	// ===================== Section-7 : Execution engine =====================
	char tape[TAPE_SIZE];
	char input[LINE_SIZE];

	while (1)
	{
		if (cli_input == NULL)
		{
			printf("\n========================================\n");
			printf("Run the machine\n");
			printf("========================================\n");
		}

		// ---- reading the input string ----
		if (cli_input != NULL)
		{
			// given with --input: a mistake here is an error, there is nobody to ask again
			if (strlen(cli_input) > MAX_INPUT)
			{
				fprintf(stderr, "timsim: input string too long (max %d characters).\n", MAX_INPUT);
				return 2;
			}
			for (size_t k = 0; cli_input[k] != '\0'; k++)
			{
				if (!isgraph((unsigned char)cli_input[k]) || cli_input[k] == BLANK)
				{
					fprintf(stderr, "timsim: invalid character at position %d of the input. Use visible characters only, no spaces and no blank '%c'.\n",
							(int)k + 1, BLANK);
					return 2;
				}
			}
			strcpy(input, cli_input);
		}
		else
		{
			while (1)
			{
				int ok = 1;
				printf("Enter input string (no spaces, no '%c', max %d chars, Enter for empty tape): ", BLANK, MAX_INPUT);
				if (fgets(line, sizeof(line), stdin) == NULL)
				{
					printf("\nInput ended unexpectedly.\n");
					return 1;
				}
				len = strlen(line);
				if (len == sizeof(line) - 1 && line[len - 1] != '\n')
				{
					while ((ch = getchar()) != '\n' && ch != EOF);
					printf("Input too long (max %d characters). Try again.\n", MAX_INPUT);
					continue;
				}
				line[strcspn(line, "\r\n")] = '\0';
				len = strlen(line);
				if (len > MAX_INPUT)
				{
					printf("Input too long (max %d characters). Try again.\n", MAX_INPUT);
					continue;
				}
				for (size_t k = 0; k < len; k++)
				{
					if (!isgraph((unsigned char)line[k]) || line[k] == BLANK)
					{
						printf("Invalid character at position %d. Use visible characters only, no spaces and no blank '%c'.\n",
								(int)k + 1, BLANK);
						ok = 0;
						break;
					}
				}
				if (!ok) continue;
				strcpy(input, line);
				break;
			}
		}

		// ---- asking for trace (skipped when --trace or --input was used) ----
		int show_trace = cli_trace;
		while (cli_input == NULL && !cli_trace)
		{
			printf("Show step by step trace? (y/n): ");
			if (fgets(line, sizeof(line), stdin) == NULL)
			{
				printf("\nInput ended unexpectedly.\n");
				return 1;
			}
			if (strlen(line) == sizeof(line) - 1 && line[sizeof(line) - 2] != '\n')
				while ((ch = getchar()) != '\n' && ch != EOF);
			if ((line[0] == 'y' || line[0] == 'Y') && line[strspn(line + 1, " \t\r\n") + 1] == '\0')
			{
				show_trace = 1;
				break;
			}
			if ((line[0] == 'n' || line[0] == 'N') && line[strspn(line + 1, " \t\r\n") + 1] == '\0')
			{
				show_trace = 0;
				break;
			}
			printf("Please answer y or n.\n");
		}

		// ---- initializing tape, head and state ----
		memset(tape, BLANK, TAPE_SIZE);
		len = strlen(input);
		for (size_t k = 0; k < len; k++)
			tape[TAPE_START + k] = input[k];

		int head = TAPE_START;
		int state = start_state;
		int steps = 0;
		int lo = head;                                  // leftmost cell visited or filled
		int hi = (len > 0) ? head + (int)len - 1 : head; // rightmost cell visited or filled
		int last_used = 0;                              // transition used in the last step
		int result = 0; // 1 accept, 2 reject state, 3 no transition, 4 step limit, 5 tape edge
		int missing_symbol = BLANK;

		if (cli_input != NULL && !quiet)
			printf("Input string : %s\n", len > 0 ? input : "(empty)");

		if (show_trace)
		{
			printf("\nExecution trace ([x] marks the head)\n");
			printf("----------------------------------------\n");
		}

		while (1)
		{
			// ===================== Section-8 : Output (trace) =====================
			if (show_trace)
			{
				printf("Step %5d | state %2d | ", steps, state);
				for (int p = lo; p <= hi; p++)
				{
					if (p == head) printf("[%c]", tape[p]);
					else printf(" %c ", tape[p]);
				}
				if (steps > 0) printf("  (transition %d)", last_used);
				printf("\n");
			}

			// halting checks
			int halted = 0;
			for (int k = 0; k < accept_count; k++)
				if (accept[k] == state) { result = 1; halted = 1; }
			for (int k = 0; k < reject_count; k++)
				if (reject[k] == state) { result = 2; halted = 1; }
			if (halted) break;

			if (steps >= step_limit)
			{
				result = 4;
				break;
			}

			// finding the matching transition
			int found = -1;
			for (int i = 0; i < n; i++)
			{
				if (t[i].from == state && t[i].read == tape[head])
				{
					found = i;
					break;
				}
			}
			if (found < 0)
			{
				result = 3;
				missing_symbol = tape[head];
				break;
			}

			// moving the head (checking the tape edge first)
			int new_head = head;
			if (t[found].move == 'L') new_head = head - 1;
			else if (t[found].move == 'R') new_head = head + 1;
			if (new_head < 0 || new_head >= TAPE_SIZE)
			{
				result = 5;
				break;
			}

			// applying the transition
			tape[head] = t[found].write;
			state = t[found].to;
			head = new_head;
			steps++;
			last_used = found + 1;
			if (head < lo) lo = head;
			if (head > hi) hi = head;
		}

		// final tape without the blanks around it
		int first = lo;
		int last = hi;
		while (first <= last && tape[first] == BLANK) first++;
		while (last >= first && tape[last] == BLANK) last--;

		// ===================== Section-9 : Output (result) =====================
		if (cli_input != NULL && quiet)
		{
			// two lines only: the verdict, then the final tape (easy to use in scripts)
			if (result == 1) printf("ACCEPTED\n");
			else if (result == 2 || result == 3) printf("REJECTED\n");
			else if (result == 4) printf("STEP_LIMIT\n");
			else printf("TAPE_LIMIT\n");
			if (first > last)
				printf("\n");
			else
			{
				for (int p = first; p <= last; p++) printf("%c", tape[p]);
				printf("\n");
			}
		}
		else
		{
			printf("\n========================================\n");
			if (result == 1)
				printf("RESULT: ACCEPTED (reached accepting state %d)\n", state);
			else if (result == 2)
				printf("RESULT: REJECTED (reached rejecting state %d)\n", state);
			else if (result == 3)
				printf("RESULT: REJECTED (halted: no transition for state %d reading '%c')\n", state, missing_symbol);
			else if (result == 4)
			{
				printf("RESULT: STEP LIMIT REACHED (%d steps) - no decision made\n", step_limit);
				printf("        The machine may be looping forever, or it may need a bigger limit.\n");
			}
			else
				printf("RESULT: STOPPED (the tape boundary was reached, tape is limited to %d cells each way)\n", TAPE_START);

			printf("Steps taken : %d\n", steps);
			printf("Final state : %d\n", state);
			printf("Head position (0 = first input cell) : %d\n", head - TAPE_START);
			printf("Final tape  : ");
			if (first > last)
				printf("(empty)");
			else
				for (int p = first; p <= last; p++) printf("%c", tape[p]);
			printf("\n========================================\n");
		}

		// with --input the program runs once and reports through its exit status
		if (cli_input != NULL)
		{
			if (result == 1) return 0;
			if (result == 2 || result == 3) return 1;
			return 3;
		}

		// ---- run another input on the same machine? ----
		int again = 0;
		while (1)
		{
			printf("\nRun another input on the same machine? (y/n): ");
			if (fgets(line, sizeof(line), stdin) == NULL)
			{
				printf("\n");
				return 0;
			}
			if (strlen(line) == sizeof(line) - 1 && line[sizeof(line) - 2] != '\n')
				while ((ch = getchar()) != '\n' && ch != EOF);
			if ((line[0] == 'y' || line[0] == 'Y') && line[strspn(line + 1, " \t\r\n") + 1] == '\0')
			{
				again = 1;
				break;
			}
			if ((line[0] == 'n' || line[0] == 'N') && line[strspn(line + 1, " \t\r\n") + 1] == '\0')
			{
				again = 0;
				break;
			}
			printf("Please answer y or n.\n");
		}
		if (!again) break;
	}

	printf("\nThank you for using TimSim.\n");
	return 0;
}
