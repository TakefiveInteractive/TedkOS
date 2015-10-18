#define RTC_ADD 0
#define RTC_REMOVE 1
#define RTC_FIND 2
#define RTC_SYNC 3

struct mp1_blink_struct {
  unsigned short location;
  char on_char; 
  char off_char;
  unsigned short on_length;
  unsigned short off_length;
  unsigned short countdown;
  unsigned short status;
  struct mp1_blink_struct* next;
} __attribute((packed)); 
