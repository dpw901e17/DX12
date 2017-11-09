#pragma once
//************************************************************************************************************** //
//** based on https://msdn.microsoft.com/en-us/library/aa390423%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396 ** //
//************************************************************************************************************** //

#include <string>
#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include <vector>
#include <map>

#define WMIA_OUTPUT_SEPARATOR L"; "



struct WMIProperty
{
	std::string name;
	std::string value;
};

class WMIDataItem
{
private:
	//contains a mapping of ([property name], [property value])
	std::vector<WMIProperty> data;
public:

	WMIDataItem() {};
	void Add(const std::string& propName, const std::string& propVal);
	std::string GetName(size_t index);
	std::string GetValue(size_t index);
	size_t Size();
};

class WMIDataCollection
{
private:
	std::vector<WMIDataItem> items;
public:
	void Add(WMIDataItem item);
	std::string MakeString(const std::vector<std::string> propNameOrder, const std::string& separator) const;
};

// Comment to linker to depend on this library
#pragma comment(lib, "wbemuuid.lib")

typedef  void(*DataItemDelegate)(const std::string* propDataArr, WMIDataItem* toFill);

class WMIAccessor {
private:
	IWbemServices *pSvc;
	IWbemLocator *pLoc;

public:
	//"Root\" should not be part of wmiNamespace! 
	WMIAccessor() {};
	void Connect(const bstr_t& wmiNamespace);
	~WMIAccessor();

	//Executes: SELECT [wmiProperties] FROM [wmiClass] and returnes the result in a csv string
	std::wstring Query(const bstr_t& wmiClass, const bstr_t wmiProperties[], int arrayCount);
	WMIDataItem QueryItem(const bstr_t& wmiClass, const bstr_t wmiProperties[], const int arrayCount, DataItemDelegate fun);
};