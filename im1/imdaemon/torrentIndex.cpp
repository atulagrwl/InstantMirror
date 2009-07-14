#include "torrentIndex.h"
#include "daemon.h"
#include "imdaemon.h"

torrentIndex::torrentIndex(const std::string str)
{
    torrentPath.assign(str);
    fileNumber = 0;
    if(torrentPath.at(torrentPath.length()-1) != '/')
	torrentPath.append("/");
}

void torrentIndex::init()
{
    int index = torrentPath.find("/");
    torrentName = torrentPath.substr(0,index);
    torrentFilePath = torrentPath.substr(index+1);
    indexstr.clear();
    indexstr.append("<!-- Created by imdaemon --> \n");
    addTitleAndHtml();
    addHeader();
    
}

void torrentIndex::addTitleAndHtml()
{
    indexstr.append("<html> \n <head> \n <title>");
    std::string tmp = "Index for " +torrentPath.substr(0,torrentPath.find("."));
    indexstr.append(tmp);
    indexstr.append("</title> \n <body> \n");
    indexstr.append("<h1>"+tmp+"</h1>");
}

void torrentIndex::addHeader()
{
    indexstr.append("<table><tr><th><img src=\"/icons/blank.gif\" alt=\"[ICO]\"width=\"20\" height=\"22\"></th><th>Name</th><th>Last modified</th><th>Size</th></tr><tr><th colspan=\"4\"><hr></th></tr>");
}

void torrentIndex::setFileList(std::vector<torfile_list> list)
{
    fileList = list;
}

void torrentIndex::generateIndex()
{
    std::string previousFile;
    std::string localstr;
    if(fileList.size() > 1)
    {
	for(int i =1 ;i< fileList.size(); i++)
	{
	    localstr = fileList[i].name;
	    int index_t = localstr.find(torrentPath);
	    if(index_t != 0)
		continue;
	    std::cout<<"localstr is "<<localstr<<" torrentPath is "<<torrentPath<<" Index_t is "<<index_t<<"\n"; 
	    localstr.assign(localstr.substr(torrentPath.length()));
	    std::cout<<"localstr is "<<localstr<<" torrentPath is "<<torrentPath<<" Index_t is "<<index_t<<"\n";
	    index_t = localstr.find('/');
	    if(index_t > 0)
	    {
		std::string substring = localstr.substr(0,index_t);
		if(previousFile.compare(substring) == 0)
		    continue;
		previousFile.assign(substring);
		addToList(T_DIRECTORY, torrentPath, substring, 0, 0);
	    }
	    else
	    {
		std::cout<<"localstr is "<<localstr<<" torrentPath is "<<torrentPath<<" Index_t is "<<index_t<<"\n";
		addToList(T_FILE, torrentPath, localstr, fileList[i].mtime, fileList[i].size);
	    }
	    
	}
    }
    else if (fileList.size() == 1)
    {
	addToList(T_FILE, "", fileList[0].name, fileList[0].mtime, fileList[0].size);
    }
    addFooter();
}

void torrentIndex::addToList(Type fileType, std::string baseurl, std::string tmp, time_t mtime, long size)
{
    struct tm *ts;
    char time_buf[18];
    ts = gmtime(&(mtime));
    
    (void) strftime( time_buf, 18, "%d-%b-%Y %H:%M", ts);
    
    char size_buf[10];
    (void) sprintf( size_buf, "%i",size);
    
    if(fileType == T_DIRECTORY)
    {
	std::cout<<"Dir is "<<tmp<<"\n";
	indexstr.append("<tr><td valign=\"top\"><img src=\"/icons/bc_folder.png\" alt=\"[DIR]\" width=\"20\"height=\"22\"></td>");
	indexstr.append("<td><a href=\"/"+ baseurl + tmp +"\">" + tmp + "</a></td><td align=\"right\">  - </td>");
	indexstr.append("<td align=\"right\">  - </td><td>&nbsp;</td></tr>\n");
    }
    else if(fileType == T_FILE)
    {
	std::cout<<"File is "<<tmp<<"\n";
	indexstr.append("<tr><td valign=\"top\"><img src=\"/icons/bc_document.png\" alt=\"[FILE]\" width=\"20\"height=\"22\"></td>");
	indexstr.append("<td><a href=\"/"+  baseurl + tmp +"\">" + tmp + "</a></td><td align=\"right\"> " + time_buf +" </td><td align=\"right\"> " + size_buf + " </td><td>&nbsp;</td></tr>\n");	
    }
}

void torrentIndex::addFooter()
{
    indexstr.append("<tr><th colspan=\"4\"><hr></th></tr> \n</table>\n");
    indexstr.append("</body>\n</html>\n");
}

std::string torrentIndex::saveIndexFile()
{
    std::string fileSavePath;
    fileSavePath.assign(DISK_LOCATION);
    fileSavePath.append(torrentPath);
    fileSavePath.append("index.html");
    
    std::ofstream fileSave(fileSavePath.c_str());
    fileSave << indexstr;
    fileSave.close();
    return torrentPath+"index.html";
}