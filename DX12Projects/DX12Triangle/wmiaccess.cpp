#include "wmiaccess.h"

WMIAccessor::WMIAccessor(const bstr_t & wmiNamespace)
{
	HRESULT hres;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		std::string msg = "Failed to initialize COM library. Error code = " + hres;
		throw std::runtime_error(msg);
		CoUninitialize();
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------

	/*
	hres = CoInitializeSecurity(
	NULL,
	-1,                          // COM authentication
	NULL,                        // Authentication services
	NULL,                        // Reserved
	RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
	RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
	NULL,                        // Authentication info
	EOAC_NONE,                   // Additional capabilities
	NULL                         // Reserved
	);


	if (FAILED(hres))
	{
	cout << "Failed to initialize security. Error code = 0x"
	<< hex << hres << endl;
	CoUninitialize();
	return 1;                    // Program has failed.
	}
	*/

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	hres = CoCreateInstance(
		CLSID_WbemLocator, // CLSID associated with data and code used to create the object.
		0,					// pointer to the aggregate object IUnknown interface (can be NULL)
		CLSCTX_INPROC_SERVER, // makes sure this process runs in our c++ context
		IID_IWbemLocator,  // Interface of output parameter
		(LPVOID *)&pLoc); // OUTPUT: Pointer to pointer to data

	if (FAILED(hres))
	{
		std::string msg = "Failed to create IWbemLocator object. Err code = " + hres;
		throw std::runtime_error(msg);
		CoUninitialize();
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t("ROOT\\" + wmiNamespace), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{

		std::string msg = "Could not connect. Error code  = " + hres;
		throw std::runtime_error(msg);
		pLoc->Release();
		CoUninitialize();
	}


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	/*
	hres = CoSetProxyBlanket(
	pSvc,                        // Indicates the proxy to set
	RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
	RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
	NULL,                        // Server principal name
	RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
	RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
	NULL,                        // client identity
	EOAC_NONE                    // proxy capabilities
	);

	if (FAILED(hres))
	{
	cout << "Could not set proxy blanket. Error code = 0x"
	<< hex << hres << endl;
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();
	return 1;               // Program has failed.
	}
	*/
}

WMIAccessor::~WMIAccessor()
{
	pSvc->Release();
	pLoc->Release();

	CoUninitialize();
}

std::wstring WMIAccessor::Query(const bstr_t & wmiClass, const bstr_t wmiProperties[], int arrayCount)
{
	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	HRESULT hres;
	IEnumWbemClassObject* pEnumerator = NULL;

	bstr_t query = "SELECT ";
	bool isFirstRun = true;
	for (auto i = 0; i < arrayCount; ++i) {
		if (!isFirstRun) {
			query += ", ";
		}
		else {
			isFirstRun = false;
		}
		query += wmiProperties[i];
	}

	query += " FROM " + wmiClass;

	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t(query),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		std::string msg = "Query for operating system name failed. Error code  = " + hres;
		throw std::runtime_error(msg);

		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject *pclsObj = NULL;
	std::wstring result;

	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}


		// Get the value of the Identifier property
		_variant_t vtProp;

		isFirstRun = true;
		for (auto i = 0; i < arrayCount; ++i) {
			auto p = wmiProperties[i];


			hr = pclsObj->Get(p,
				0,
				&vtProp,
				0, //<--- CIM_EMPTY
				0);

			//force type to be BSTR (if actual value is a number n, then this call makes it "n")
			vtProp.ChangeType(VT_BSTR);

			//append WMIA_OUTPUT_SEPERATOR only if we had previous data in result
			if (!isFirstRun) {
				result += WMIA_OUTPUT_SEPARATOR;
			}
			isFirstRun = false;

			result += vtProp.bstrVal;
			isFirstRun = false;

			VariantClear(&vtProp);
		}

		pclsObj->Release();
		result += L"\n";
	}

	pEnumerator->Release();
	return result;
}

WMIDataItem WMIAccessor::QueryItem(const bstr_t & wmiClass, const bstr_t wmiProperties[], const int arrayCount, DataItemDelegate fun)
{
	WMIDataItem result;

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	HRESULT hres;
	IEnumWbemClassObject* pEnumerator = NULL;

	bstr_t query = "SELECT ";
	bool isFirstRun = true;
	for (auto i = 0; i < arrayCount; ++i) {
		if (!isFirstRun) {
			query += ", ";
		}
		else {
			isFirstRun = false;
		}
		query += wmiProperties[i];
	}

	query += " FROM " + wmiClass;

	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t(query),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		std::string msg = "Query for operating system name failed. Error code  = " + hres;
		throw std::runtime_error(msg);

		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject *pclsObj = NULL;

	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}


		std::vector<std::string> propertyData;

		// Get the value of the Identifier property
		_variant_t vtProp;

		for (auto i = 0; i < arrayCount; ++i) {
			auto p = wmiProperties[i];


			hr = pclsObj->Get(p,
				0,
				&vtProp,
				0, //<--- CIM_EMPTY
				0);

			//force type to be BSTR (if actual value is a number n, then this call makes it "n")
			vtProp.ChangeType(VT_BSTR);
			std::string propertyDataVal = _bstr_t(vtProp.bstrVal);

			propertyData.push_back(propertyDataVal);

			VariantClear(&vtProp);
		}

		pclsObj->Release();
		fun(propertyData.data(), &result);
	}

	pEnumerator->Release();
	return result;
}

void WMIDataItem::Add(const std::string & propName, const std::string & propVal)
{
	data.push_back(WMIProperty{ propName, propVal });
}

std::string WMIDataItem::GetName(size_t index)
{
	return data.at(index).name;
}

std::string WMIDataItem::GetValue(size_t index)
{
	return data.at(index).value;
}

size_t WMIDataItem::Size()
{
	return data.size();
}

void WMIDataCollection::Add(WMIDataItem item)
{
	items.push_back(item);
}

std::string WMIDataCollection::MakeString(const  std::vector<std::string> propNameOrder, const std::string& separator) const
{
	//make header:
	std::string result = "id";

	for (auto i = 0; i < propNameOrder.size(); ++i) {
		result += separator + " ";
		result += propNameOrder[i];
	}

	//end header
	result += "\n";

	//for each item in collection
	for (auto itemIndex = 0; itemIndex < items.size(); ++itemIndex) {
		auto item = items.at(itemIndex);

		//create a row for each property
		for (auto row = 0; row < item.Size() / propNameOrder.size(); ++row) {

			//add ID
			result += std::to_string(itemIndex);

			//look at order:
			for (auto orderIndex = 0; orderIndex < propNameOrder.size(); ++orderIndex) {
				//search for collumn with property name matching the current order (at current row)
				bool propertyFound = false;
				auto col = 0;

				while (!propertyFound && col < propNameOrder.size()) {
					propertyFound = item.GetName(row * propNameOrder.size() + col) == propNameOrder[orderIndex];
					if (propertyFound) {
						result += separator + " ";
						result += item.GetValue(row * propNameOrder.size() + col);
					}
					else {
						if (!propertyFound && col == propNameOrder.size() - 1)
						{
							//this occurs when data NOT from the WMI query is added to the item (e.g. timestamp)
							//since the item is created through QueryItem, then anything added to it
							//must have happened after the query has been processes => it is at the end
							//of the item.data set. Search for it, beginning from the end:
							bool found = false;
							auto index = item.Size() - 1;
							while (!found && index >= 0) {
								found = item.GetName(index) == propNameOrder[orderIndex];
								if (found) {
									result += separator + " ";
									result += item.GetValue(index);
								}
								else {
									--index;
								}
							}
						}
						++col;
					}
				}
			}
			result += "\n";
		}
	}

	return result;
}
