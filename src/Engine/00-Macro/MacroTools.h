#pragma once

#ifndef __FILENAME__
#define __FILENAME__ \
	(strrchr(__FILE__,'/') \
	? strrchr(__FILE__,'/')+1 \
	: (strrchr(__FILE__,'\\') \
	? strrchr(__FILE__,'\\')+1 \
	: __FILE__ \
	))
#endif
