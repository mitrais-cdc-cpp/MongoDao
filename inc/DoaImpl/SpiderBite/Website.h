/*
 * Website.h
 *
 *  Created on: Jul 4, 2016
 *      Author: Ari Suarkadipa
 */

#ifndef INC_WEBSITE_H_
#define INC_WEBSITE_H_

#include <ctime>

using namespace std;

namespace Mitrais
{
	namespace util
	{
		// struct of Website
		struct Website{
			string 	content;
			string 	protocolType;
			time_t 	createdDate;
			time_t 	lastUpdated;
		};
	}
}

#endif /* INC_WEBSITE_H_ */
