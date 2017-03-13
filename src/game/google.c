#include "google.h"
#include "cfg/cfg.h"

#if PP_ANDROID
	/* Needed only for sendScore() */
	#include <SDL.h>
	#include <jni.h>
#endif

static int Android_BooleanVoidFn(const char *func)
{
#if PP_ANDROID
	JNIEnv *env;
	jobject theActivity;
	jclass theClass;
	jmethodID theMethod;
	jboolean retval;

	env = (JNIEnv *) SDL_AndroidGetJNIEnv();
	theActivity = (jobject) SDL_AndroidGetActivity();

	theClass = (*env)->GetObjectClass(env, theActivity);
	theMethod = (*env)->GetStaticMethodID(env, theClass, func, "()Z");
	retval = (*env)->CallStaticBooleanMethod(env, theClass, theMethod);

	(*env)->DeleteLocalRef(env, theActivity);
	(*env)->DeleteLocalRef(env, theClass);
	return retval == JNI_TRUE;
#else
	return 0;
#endif
}

int Android_IsConnectedToGooglePlay(void)
{
	return Android_BooleanVoidFn("isConnectedToGooglePlay");
}

int Android_IsConnectingToGooglePlay(void)
{
	return Android_BooleanVoidFn("isConnectingToGooglePlay");
}

int Android_IsRequestingLeaderboard(void)
{
	return Android_BooleanVoidFn("isRequestingLeaderboard");
}

void Android_ConnectToGooglePlay(void)
{
#if PP_ANDROID
	JNIEnv *env;
	jobject theActivity;
	jclass theClass;
	jmethodID theMethod;

	env = (JNIEnv *) SDL_AndroidGetJNIEnv();
	theActivity = (jobject) SDL_AndroidGetActivity();

	theClass = (*env)->GetObjectClass(env, theActivity);
	theMethod = (*env)->GetStaticMethodID(env, theClass,
		       	"connectToGooglePlay", "()V");
	(*env)->CallStaticVoidMethod(env, theClass, theMethod);

	(*env)->DeleteLocalRef(env, theActivity);
	(*env)->DeleteLocalRef(env, theClass);
#endif
}

void Android_SendScore(const char *boardId, const char *score)
{
#if PP_ANDROID
	JNIEnv *env;
	jobject theActivity;
	jclass theClass;
	jmethodID theMethod;
	jstring scorej, boardj;

	env = (JNIEnv *) SDL_AndroidGetJNIEnv();
	theActivity = (jobject) SDL_AndroidGetActivity();

	theClass = (*env)->GetObjectClass(env, theActivity);
	theMethod = (*env)->GetStaticMethodID(env, theClass, "sendScore",
			"(Ljava/lang/String;Ljava/lang/String;)V");
	boardj = (*env)->NewStringUTF(env, boardId); 
	scorej = (*env)->NewStringUTF(env, score);
	if (scorej != NULL && boardj != NULL) {
		(*env)->CallStaticVoidMethod(env, theClass, theMethod,
			boardj, scorej);
	}
	if (scorej != NULL) {
		(*env)->DeleteLocalRef(env, scorej);
	}
	if (boardj != NULL) {
		(*env)->DeleteLocalRef(env, boardj);
	}

	(*env)->DeleteLocalRef(env, theActivity);
	(*env)->DeleteLocalRef(env, theClass);
#endif
}

void Android_ShowLeaderboard(const char *boardId)
{
#if PP_ANDROID
	JNIEnv *env;
	jobject theActivity;
	jclass theClass;
	jmethodID theMethod;
	jstring boardj;

	env = (JNIEnv *) SDL_AndroidGetJNIEnv();
	theActivity = (jobject) SDL_AndroidGetActivity();

	theClass = (*env)->GetObjectClass(env, theActivity);
	theMethod = (*env)->GetStaticMethodID(env, theClass, "showLeaderboard",
			"(Ljava/lang/String;)V");
	boardj = (*env)->NewStringUTF(env, boardId); 
	if (boardj != NULL) {
		(*env)->CallStaticVoidMethod(env, theClass, theMethod, boardj);
		(*env)->DeleteLocalRef(env, boardj);
	}

	(*env)->DeleteLocalRef(env, theActivity);
	(*env)->DeleteLocalRef(env, theClass);
#endif
}
