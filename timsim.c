
//source code - turing machine simulator(TimSim)
//Author - Krishna Singh
//Phase 1 : Transition input   | Phase 2 : Validation   | Phase 3 : Machine execution
#include <stdio.h>
#include <string.h>
#include <ctype.h>
 
// ===================== Section-1 : Constants and data structures =====================
#define MAX_TRANSITIONS 30
#define MAX_STATE 99          // states are whole numbers from 0 to MAX_STATE
#define MAX_ACCEPT 10         // maximum number of accepting states
#define MAX_REJECT 10         // maximum number of rejecting states
#define MAX_INPUT 200         // maximum length of the input string
#define MAX_STEP_LIMIT 100000 // largest step limit the user can choose
#define DEFAULT_STEP_LIMIT 1000
#define BLANK '_'             // the blank symbol of the tape
#define TAPE_SIZE 20001       // fixed tape size (the head can never go past its ends, see Section-7)
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
 
int main()
{
	// ===================== Section-2 : Configuration =====================
	int n; // number of transitions
	delta t[MAX_TRANSITIONS];
 
	int start_state;
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
 
	printf("========================================\n");
	printf("   TimSim - Turing Machine Simulator\n");
	printf("========================================\n");
	printf("Rules used by this simulator:\n");
	printf(" - States are whole numbers from 0 to %d\n", MAX_STATE);
	printf(" - Tape symbols are any visible character (no spaces), blank symbol is '%c'\n", BLANK);
	printf(" - The machine is deterministic (one transition per state + read symbol)\n");
	printf(" - Halts when it reaches an accepting or rejecting state, or when no\n");
	printf("   transition exists for the current state and symbol (counts as reject)\n");
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
 
	// step limit (optional)
	while (1)
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
 
	// ===================== Section-6 : State validation =====================
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
		printf("Warning: start state %d has no outgoing transition, the machine will halt immediately.\n", start_state);
		warnings++;
	}
 
	for (int k = 0; k < accept_count; k++)
	{
		int reachable = (accept[k] == start_state);
		for (int i = 0; i < n; i++)
			if (t[i].to == accept[k]) reachable = 1;
		if (!reachable)
		{
			printf("Warning: accepting state %d is never used as a next state, it can never be reached.\n", accept[k]);
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
			printf("Warning: transition %d starts from halting state %d, it will never be used.\n", i + 1, t[i].from);
			warnings++;
		}
	}
 
	printf("\nState validation passed with %d warning(s).\n", warnings);
	printf("Start state: %d | Accepting: ", start_state);
	for (int k = 0; k < accept_count; k++) printf("%d ", accept[k]);
	printf("| Rejecting: ");
	if (reject_count == 0) printf("none (a missing transition rejects) ");
	for (int k = 0; k < reject_count; k++) printf("%d ", reject[k]);
	printf("| Step limit: %d\n", step_limit);
 
	// ===================== Section-7 : Execution engine =====================
	char tape[TAPE_SIZE];
	char input[LINE_SIZE];
 
	while (1)
	{
		printf("\n========================================\n");
		printf("Run the machine\n");
		printf("========================================\n");
 
		// reading the input string
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
 
		// asking for trace
		int show_trace = 0;
		while (1)
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
 
		// initializing tape, head and state
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
 
		// ===================== Section-8 : Output (result) =====================
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
 
		// final tape without the blanks around it
		int first = lo;
		int last = hi;
		while (first <= last && tape[first] == BLANK) first++;
		while (last >= first && tape[last] == BLANK) last--;
		printf("Final tape  : ");
		if (first > last)
			printf("(empty)");
		else
			for (int p = first; p <= last; p++) printf("%c", tape[p]);
		printf("\n========================================\n");
 
		// run again?
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
