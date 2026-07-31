# Correspondence mode (v2, behind a feature flag)

Correspondence play is out of the default build; this documents the intended
protocol so v2 lands on a settled design. Nothing below is compiled unless
`-DCHESSINK_CAP_CORRESPONDENCE=1` is set (no code ships in v1).

## Protocol

One game = one plain-text file on an HTTP endpoint both players can reach
(the DailyDrop/InkWriter convention: any static host works).

```
CGAME 1
M game chess
M white mo
M black alex
V e2e4
V e7e5
```

- The file only ever grows: each player appends one `V <move>` line.
- Polling: the device fetches the file over Wi-Fi (explicit sync action,
  same radio discipline as the other ecosystem apps), validates the whole
  line with the on-device engine, and if it is your turn, lets you pick a
  move and PUT the extended file back.
- Conflict safety: the PUT includes an `If-Match`/ETag when the server
  offers one; otherwise last-writer-wins is accepted for the hobby case and
  the full move-list validation makes a torn write detectable (an illegal
  or shortened file is refused and refetched).

## Why deferred

Wi-Fi + the puzzle UI is fine RAM-wise, but the mode needs account/pairing
UX and abuse-resistant polling schedules that deserve their own design
pass; shipping the format first lets the companion grow a relay before the
firmware commits.
