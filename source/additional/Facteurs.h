struct s_state {
	int label;
	int p;
	int l;
	struct s_state **S;
	struct s_transition **out;
	};
typedef struct s_state p_state;

struct s_transition {
	int label;
	struct s_state **out;
	int offset;
	struct s_transition **next;
	};
typedef struct s_transition p_transition;