/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway software UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: version.h,v 5.4 2006/05/18 18:35:08 anray Exp $
 *
 * Global version number, patch level
 *****************************************************************************/

#ifndef FIDOGATE_VERSION_H_
#define FIDOGATE_VERSION_H_

#include <config.h>
#define VERSION_MAJOR	5
#define VERSION_MINOR	2
#define PATCHLEVEL	5
#define EXTRAVERSION	"-g" GIT_HASH_STR

#define STATE		"unstable"

#endif
