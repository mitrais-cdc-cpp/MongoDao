/*
 * Connector.cpp
 *
 *  Created on: Jul 1, 2016
 *      Author: Ari Suarkadipa
 */

#include "../inc/DoaImpl/SpiderBite/Connector.h"

using namespace Mitrais::util;

// Global static pointer used to ensure a single instance of the class.
Connector* Connector::m_pInstance = NULL;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/
Connector* Connector::Instance()
{
	if (!m_pInstance)
	{
		m_pInstance = new Connector;
	}

	return m_pInstance;
}

/**
 * Insert Website to MongoDB
 *
 * @param website is a website data to insert
 */
void Connector::Insert(Website &website) {
	DateTime now;
	MongoDB::Document::Ptr websitePtr = new MongoDB::Document();

	websitePtr->add("content", website.content);
	websitePtr->add("protocolType", website.protocolType);
	websitePtr->add("createdDate", now.timestamp());
	websitePtr->add("lastUpdated", MongoDB::NullValue());

	MongoDB::Database db(_dbName);
	SharedPtr<MongoDB::InsertRequest> insertRequest = db.createInsertRequest(
			_collectionName);
	insertRequest->documents().push_back(websitePtr);

	MongoDB::Connection mongo;
	mongo.connect(_host, _port);

	mongo.sendRequest(*insertRequest);
}

/**
 * Update MongoDB
 *
 * @param db a db name
 * @param whereColumn a column
 * @param whereValue a condition
 * @param newValue a new value
 */
void Connector::Update(MongoDB::Database &db,
		string whereColumn, string whereValue, string newValue) {
	SharedPtr<MongoDB::UpdateRequest> updateRequest = db.createUpdateRequest(
			_collectionName);

	MongoDB::Document& selector = updateRequest->selector();
	MongoDB::Document& update = updateRequest->update();

	selector.add(whereColumn, whereValue);
	update.addNewDocument("$set").add(whereColumn, newValue);

	MongoDB::Connection mongo;
	mongo.connect(_host, _port);

	mongo.sendRequest(*updateRequest);
}

/**
 * Delete record on MongoDB
 *
 * @param filters a filter
 */
void Connector::Delete(vector<Filter> &filters)
{
	MongoDB::Database db(_dbName);
	SharedPtr<MongoDB::DeleteRequest> deleteRequest = db.createDeleteRequest(_collectionName);

	if (!filters.empty())
	{
		MongoDB::Document& selector = deleteRequest->selector();

		for (vector<Filter>::iterator it = filters.begin(); it != filters.end();
				++it) {
			string field = it->field;
			string value = it->value;
			Opr op = it->op;

			cout << "Field " << field << endl;
			cout << "Value " << value << endl;

			if (op == EQUALS)
			{
				selector.add(field, value);
			}
			else
			{

				break;
				switch (op)
				{
					case NOT_EQUALS:
					{
						selector.addNewDocument(field).add("$ne", value);
					}
					break;
					case GREATER_THAN:
					{
						selector.addNewDocument(field).add("$gt", value);
					}
					break;
					case GREATER_THAN_EQUALS:
					{
						selector.addNewDocument(field).add("$gte", value);
					}
					break;
					case LESS_THAN:
					{
						selector.addNewDocument(field).add("$lt", value);
					}
					break;
					case LESS_THAN_EQUALS:
					{
						selector.addNewDocument(field).add("$lte", value);
					}
					break;
					case IN:
					{
						selector.addNewDocument(field).add("$in", value);
					}
					break;
					case NOT_IN:
					{
						selector.addNewDocument(field).add("$nin", value);
					}
					break;
				}
			}
		}
	}

	MongoDB::Connection mongo;
	mongo.connect(_host, _port);

	mongo.sendRequest(*deleteRequest);
}

/**
 * Get all record from MongoDB
 *
 * @param response a response
 * @return template a template
 */
template<typename T>
vector<T> Connector::GetAll(MongoDB::ResponseMessage &response) {
	vector<T> collection;
	try{
		int size = response.documents().size();
		for (int i = 0; i < size; i++) {
			T obj;
			MongoDB::Document::Ptr doc = response.documents()[i];

			obj.content = doc->get<string>("content");
			obj.protocolType = doc->get<string>("protocolType");

			collection.push_back(obj);
		}
	}
	catch (const std::bad_alloc& e) {
		std::cout << "Allocation failed: " << e.what() << '\n';
	}

	return collection;
}

/**
 * Show all records from MongoDB
 */
void Connector::showAll() {
	cout << "------------- Website Data --------------" << endl;
	cout << "No.   |   Content   |   Protocol Type   |" << endl;
	cout << "-----------------------------------------" << endl;

	try {
		MongoDB::QueryRequest request(_dbName + "." + _collectionName);
		MongoDB::ResponseMessage response;

		MongoDB::Connection mongo;
		mongo.connect(_host, _port);

		mongo.sendRequest(request, response);

		vector<Website> websites = GetAll<Website>(response);
		int size = websites.size();

		for (int i = 0; i < size; i++) {
			Website obj = websites[i];
			cout << "  " << (i + 1) << "   |   " << obj.content << "   |   "
					<< obj.protocolType << endl;
		}
	}
	catch (NotFoundException& nfe) {
		cout << nfe.message() + " not found." << endl;
	}
	catch (const std::bad_alloc& e) {
        std::cout << "Allocation failed: " << e.what() << '\n';
    }
}

/**
 * Create filter used for delete method
 *
 * @param selectedColumn
 * 1 is content
 * 2 is protocolType
 *
 * @param selectedOpr is selected operator
 * 1. Equal
 * 2. Not Equal
 * 3. Greater
 * 4. Greater Than Equals
 * 5. Less Than
 * 6. Less Than Equal
 * 7. In
 * 8. Not In
 *
 * @param value is a value criteria to delete
 *
 * @return Filter
 */
Filter Connector::createFilter(int selectedColumn, int selectedOpr, string value) {
	Filter filter;

	switch (selectedColumn)
	{
		case 1:
		{
			filter.field = "content";
		}
		break;
		case 2:
		{
			filter.field = "protocolType";
		}
	}

	switch (selectedOpr) {
	case 1: {
		filter.op = EQUALS;
	}
		break;
	case 2: {
		filter.op = NOT_EQUALS;
	}
		break;
	case 3: {
		filter.op = GREATER_THAN;
	}
		break;
	case 4: {
		filter.op = GREATER_THAN_EQUALS;
	}
		break;
	case 5: {
		filter.op = LESS_THAN;
	}
		break;
	case 6: {
		filter.op = LESS_THAN_EQUALS;
	}
		break;
	case 7: {
		filter.op = IN;
	}
		break;
	case 8: {
		filter.op = NOT_IN;
	}
	}

	filter.value = value;

	return filter;
}

/**
 * Check the record is Database
 *
 * @param content a content to search
 */
bool Connector::checkIsExist(string content)
{
	bool result = false;
	MongoDB::Connection mongo;
	MongoDB::QueryRequest request(_dbName + "." + _collectionName);
	MongoDB::ResponseMessage response;

	mongo.connect(_host, _port);
	mongo.sendRequest(request, response);

	if ( response.documents().size() > 0 )
	{
		int size = response.documents().size();
		for (int i = 0 ; i < size ; i++)
		{
			Poco::MongoDB::Document::Ptr doc = response.documents()[i];

			try
			{
				string contentName = doc->get<string>("content");

				if (contentName == content)
				{
					cout << "Content " << content << " is exist on our database."<<endl;
					result = true;
				}
			}
			catch(Poco::Exception& ex)
			{
				cout << ex.message();
			}
		}
	}
	else
	{
		cout << "There are no Website data " << content << " on our database."<<endl;
	}

	return result;
}


