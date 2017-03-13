#ifndef GOOGLE_H
#define GOOGLE_H

int Android_IsConnectedToGooglePlay(void);
int Android_IsConnectingToGooglePlay(void);
void Android_ConnectToGooglePlay(void);
void Android_SendScore(const char *boardId, const char *score);
void Android_ShowLeaderboard(const char *boardId);
int Android_IsRequestingLeaderboard(void);

#endif
