/**
* Project: VSXu Engine: Realtime modular visual programming engine.
*
* This file is part of Vovoid VSXu Engine.
*
* @author Jonatan Wallmander, Robert Wenzel, Vovoid Media Technologies AB Copyright (C) 2003-2013
* @see The GNU Lesser General Public License (LGPL)
*
* VSXu Engine is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU Lesser General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#endif
#include <debug/vsx_error.h>
#include "vsxfst.h"

#ifdef VSX_PRINTF_TO_FILE

vsx_pf_file_holder vsx_pf_file_holder::instance;

#endif

vsx_string<>str_pad(const vsx_string<>& str, const vsx_string<>& chr, size_t t_len, int pad_type, int overflow_adjust) {
  vsx_string<>ps = "";
  if (str.size() > t_len)
  {
    if (overflow_adjust == STR_PAD_OVERFLOW_LEFT) {
      for (size_t i = 0; i < t_len; ++i) {
        ps = vsx_string<>(str[str.size()-i-1]) + ps;
      }
      return ps;
    } else
    {
      for (size_t i = 0; i < t_len; ++i) {
        ps += str[i];
      }
      return ps;
    }
  } else
  {
    for (size_t i = 0; i < (t_len-str.size()); ++i) {
      ps += chr;
    }
    if (pad_type == STR_PAD_LEFT)
    ps = ps+str;
    else
    ps = str+ps;
    if (ps.size() > t_len) return str_pad(str,chr,t_len,pad_type,overflow_adjust);
    else
    return ps;
  }
}


vsx_string<>str_replace(vsx_string<>search, vsx_string<>replace, vsx_string<>subject, int max_replacements, int required_pos) {
  //printf("------------\nsubject coming in: %s\n",subject.c_str());
  vsx_string<>n = subject;
  if (search == "")
    return n;
  int loc = 1;
  int replacements = 0;
  while ((loc = n.find(search, loc-1)) != -1) {
//      printf("loc: %d\n",loc);
    if (loc <= required_pos || required_pos == -1) {
//      printf("subject changed 0: %s\n",subject.c_str());
      if (replace.size()) {
        n = n.substr(0,loc) + replace + n.substr(loc+search.size());
        loc += replace.size();
      }
      else
      {
        n = n.substr(0,loc) + n.substr(loc+search.size());
        ++loc;
      }
//        printf("subject changed 1: %s\n",n.c_str());

    } else return n;
    if (max_replacements) {
      replacements++;
      if (replacements >= max_replacements) {
        return n;
      }
    }
  }
//  printf("reached end %s\n",n.c_str());
  return n;
}

const vsx_string<>str_replace_char_pad(vsx_string<>search, vsx_string<>replace, vsx_string<>subject, vsx_string<>subject_r, int max_replacements, int required_pos) {
  //printf("------------\nsubject coming in: %s\n",subject.c_str());
  //vsx_string<>hh = "";
  if (subject.size() != subject_r.size()) return subject_r;
  vsx_string<>rep;
  for (size_t i = 0; i < search.size(); ++i) rep.push_back(replace[0]);
  vsx_string<>n;
  //printf("first\n");
  if (search == "") return subject_r;
  int loc = 1;
  int replacements = 0;
  while ((loc = subject.find(search, loc-1)) != -1) {
  if (loc <= required_pos || required_pos == -1) {
 //      printf("loc: %d\n",loc);
      subject = subject.substr(0,loc) + rep + subject.substr(loc+search.size());
      subject_r = subject_r.substr(0,loc) + rep + subject_r.substr(loc+search.size());
         //printf("subject changed 1: %s\n",subject.c_str());
      loc += replace.size();
    } else return subject_r;
    if (max_replacements) {
      replacements++;
      if (replacements >= max_replacements) {
        return subject_r;
      }
    }
  }
  //printf("last\n");
  return subject_r;
}

int explode(vsx_string<>& input, vsx_string<>& delimiter, vsx_nw_vector< vsx_string<> >& results, int max_parts) {
  results.clear();
  if (input == delimiter) {
    results.push_back(input);
    return 1;
  }
  //printf("splitting vsx_string: %s\n",input.c_str());
  vsx_string<>res;
  size_t fpos = 0;
  int count = 0;
  char lastchar = 0;
  for (size_t i = 0; i < input.size(); ++i)
  {
    if (input[i] == delimiter[fpos] && lastchar != '\\') {
      ++fpos;
    } else {
      res.push_back(input[i]);
      fpos = 0;
    }
    if (fpos == delimiter.size() || i == input.size()-1)
    {
      fpos = 0;
      results.push_back(res);
      res = "";
      //res.clear();
      ++count;
    }
    if (count >= max_parts && max_parts > 0) return count;
    lastchar = input[i];
  }
  if (count == 0) results.push_back(input);
  return count;
}

vsx_string<>implode(vsx_nw_vector< vsx_string<> >& in, vsx_string<>& delimiter, size_t start_index, size_t shave_off_at_end)
{
  if (in.size() == 0)
    return "";

  if (in.size() == 1)
    return in[0];

  if (start_index > in.size()-1)
    VSX_ERROR_RETURN_V("start index too big", "");

  vsx_string<>h;
  for (size_t i = start_index; i < in.size()-(1 + shave_off_at_end); ++i) {
    h += in[i]+delimiter;
  }
  h += in[in.size()-1];
  return h;
}





















void str_remove_equal_prefix(vsx_string<>* str1, vsx_string<>* str2, vsx_string<>delimiter) {
  vsx_string<> to_remove = "";
  vsx_string<> deli = delimiter;
  vsx_nw_vector< vsx_string<> > str1_;
  vsx_nw_vector< vsx_string<> > str2_;
  explode(*str1, deli, str1_);
  explode(*str2, deli, str2_);
  *str1 = implode(str1_, deli, 1);
  *str2 = implode(str2_, deli, 1);
}


/*
int split_string(vsx_string<>& input, vsx_string<>& delimiter, std::vector <vsx_string<> >& results, int max_parts) {
  results.clear();
  if (input == delimiter) {
    results.push_back(input);
    return 1;
  }
  vsx_string<>res;
  size_t fpos = 0;
  int count = 0;
  char lastchar = 0;
  for (size_t i = 0; i < input.size(); ++i)
  {
    if (input[i] == delimiter[fpos] && lastchar != '\\')
    {
      ++fpos;
    } else {
      res.push_back(input[i]);
      fpos = 0;
    }
    if (fpos == delimiter.size() || i == input.size()-1)
    {
      fpos = 0;
      results.push_back(res);
      res = "";
      ++count;
    }
    if (count >= max_parts && max_parts > 0)
    {
      i++;
      while (i < input.size())
      {
        results[results.size()-1].push_back( input[i] );
        i++;
      }
      return count;
    }
    lastchar = input[i];
  }
  if (count == 0)
    results.push_back(input);
  return count;
}
int explode(vsx_string<>& input, vsx_string<>& delimiter, std::vector <vsx_string<> >& results, int max_parts)
{
  return split_string(input, delimiter,results,max_parts);
}




vsx_string<>implode(std::vector <vsx_string<> > in,vsx_string<>delimiter) {
  if (in.size() == 0) return "";
  if (in.size() == 1) return in[0];
  vsx_string<>h;
  for (std::vector <vsx_string<> >::iterator it = in.begin(); it != in.end()-1; ++it) {
    h += *it+delimiter;
  }
  h += in.back();
  return h;
}
*/
bool verify_filesuffix(vsx_string<>& input, const char* type)
{
  vsx_nw_vector <vsx_string<> > parts;
  vsx_string<>deli = ".";
  explode(input,deli,parts);
  if (parts.size()) {
    vsx_string<>a = parts[parts.size()-1];
    a.make_lowercase();
    vsx_string<>t = type;
    t.make_lowercase();
    if (t == a)
      return true;
  }
  return false;
}

#ifndef _WIN32
void get_files_recursive_(vsx_string<>startpos, std::list< vsx_string<> >* filenames,vsx_string<>include_filter,vsx_string<>exclude_filter,vsx_string<>dir_ignore_token);
void get_files_recursive(vsx_string<>startpos, std::list< vsx_string<> >* filenames,vsx_string<>include_filter,vsx_string<>exclude_filter,vsx_string<>dir_ignore_token)
{
  get_files_recursive_(startpos,filenames,include_filter,exclude_filter,dir_ignore_token);
  (*filenames).sort();
}
void get_files_recursive_(vsx_string<>startpos, std::list< vsx_string<> >* filenames,vsx_string<>include_filter,vsx_string<>exclude_filter,vsx_string<>dir_ignore_token)
{
#else
void get_files_recursive(vsx_string<>startpos, std::list< vsx_string<> >* filenames,vsx_string<>include_filter,vsx_string<>exclude_filter,vsx_string<>dir_ignore_token) {
#endif

#ifdef _WIN32
  _finddata_t fdp;
#else
  DIR* dir;
  dirent* dp;
#endif

  vsxf filesystem;

  long fhandle = 0;
  bool run = true;
#ifdef _WIN32
  vsx_string<>fstring = startpos+"/*";
  fhandle=_findfirst(fstring.c_str(),&fdp);
#else
  vsx_string<>fstring = startpos;
  dir = opendir(startpos.c_str());
  if (!dir) return;
  dp = readdir(dir);
  if (!dp) return;
#endif
  bool exclude = false;
  bool include = true;
#ifdef VSXS_DEBUG
  FILE* fp = fopen("get_files_recursive.log","a");
  fprintf(fp,"Starting traversion in : %s\n",fstring.c_str());
#endif
  if (fhandle != -1)
  while (run)
  {
    vsx_string<>cur_directory_item;
#ifdef _WIN32
    cur_directory_item = fdp.name;
#else
    cur_directory_item = dp->d_name;
    // stat the file to see if it's a dir or not
    struct stat stbuf;
    vsx_string<>full_path = fstring + "/" + cur_directory_item;
    stat(full_path.c_str(), &stbuf);
#endif
#ifdef VSXS_DEBUG
    fprintf(fp,"File found: %s\n",full_path.c_str());
#endif
    if (include_filter == "")
      include = true;
    else
    {
#ifdef _WIN32
      if (!(fdp.attrib & _A_SUBDIR))
#else
      if (!S_ISDIR(stbuf.st_mode) && !S_ISLNK(stbuf.st_mode))
#endif
        include = false;
        if (cur_directory_item.find(include_filter) != -1)
          include = true;
    }

    if (exclude_filter == "")
    {
      exclude = false;
    }
    else {
      vsx_nw_vector< vsx_string<> > parts;
      vsx_string<>deli = " ";
      explode(exclude_filter,deli, parts);
      exclude = false;
      unsigned long i = 0;
      while (i < parts.size() && !exclude)
      {
        if (cur_directory_item.find(parts[i]) != -1) exclude = true;
        ++i;
      }
    }

    if ((cur_directory_item != ".") && (cur_directory_item != "..") && !exclude)
    {
      #ifdef VSXS_DEBUG
        fprintf(fp,"ss = %s  ___ include %d exclude %d  link: %d \n",ss.c_str(),include,exclude,(int)(stbuf.st_mode));
        fprintf(fp,"S_ISREG: %d\n", S_ISREG(stbuf.st_mode) );
        fprintf(fp,"S_ISDIR: %d\n", S_ISDIR(stbuf.st_mode) );
      #endif
#ifdef _WIN32
      if (fdp.attrib & _A_SUBDIR)
#else
      if (S_ISDIR(stbuf.st_mode) || S_ISLNK(stbuf.st_mode))
#endif
      {
        if (!filesystem.is_file( startpos + "/" + cur_directory_item + "/" + dir_ignore_token) )
        {
          // recurse into the subdirectory
          get_files_recursive(startpos+"/"+cur_directory_item,filenames,include_filter,exclude_filter,dir_ignore_token);
        }
      } else {
        if (include)
        {
          filenames->push_back(startpos+"/"+cur_directory_item);
          //printf("adding %s\n", vsx_string<>(startpos+"/"+ss).c_str());
        }
      }
    }
#ifdef _WIN32
    if (_findnext(fhandle,&fdp) == -1) run = false;
#else
  dp = readdir(dir);
    if (dp == 0)
      run = false;
#endif
  }
#ifdef _WIN32
  _findclose(fhandle);
#else
  closedir(dir);
#endif
  #ifdef VSXS_DEBUG
  fclose(fp);
  #endif
}

