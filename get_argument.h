/**@file    GET_ARGUMENTS.H
 * @brief   Provides functions to parse and get command line arguments
 * @author  John P. Hilbert
 * @email   jphilbert@gmail.com
 * @date    1/18/2025
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GET_ARGUMENTS_H
#define GET_ARGUMENTS_H

// ------------------------------------------------------------------------- //
#include <string>
#include <regex>

#define QUOTE_REGEX     L"((?:^|[^\\\\])(?:\\\\{2})*)\""
#define WORD_REGEX      L"[^\\s=]+"

using namespace std;


/**@brief  Splits a string into a list of arguments
 * 
 * In the simplest form this function splits a string at word boundaries (i.e.
 * \s) preserving the white spaces. The more complex cases arise with double
 * quotes which, if not escaped, denote a full argument. Escaped quotes and
 * quotes within words are ignored regardless. In all cases, the parsing is
 * lossless allowing reconstruction of the input.
 *
 * Example:
 * 'arg1  arg2 "arg 3"   arg"4' -->
 *         ['arg1',
 *          '  ',
 *          'arg2',
 *          ' ',
 *          '"arg 3"',
 *          '   ',
 *          'arg"4']
 * 
 * @param  ARG_LINE:    command line to parse, i.e. GetCommandLineW()
 * 
 * @return vector of strings including the whitespaces
 */
vector<wstring> ParseArguments (wstring arg_line) {
  // Initialize the parsed list of strings to be returned
  vector<wstring>
    output          = {};

  // Regex and Iterators
  wregex
    regex_word(WORD_REGEX),
    regex_quote(QUOTE_REGEX);

  regex_iterator<wstring::iterator>
    iter_word(arg_line.begin(), arg_line.end(), regex_word),
    iter_quote(arg_line.begin(), arg_line.end(), regex_quote),
    iter_end;

  // Cursor positions as we step through the string
  wstring::size_type
    // beginning of word
    pos_word_0      = 0,
    // end of word
    pos_word_1      = 0,
    // beginning of quote
    pos_quote       = iter_quote->position() + iter_quote->length() - 1;
  
  // Quote count (though mainly used to identify if within quotes)
  int
    cnt_quote       = 0;


  // Iterate through each word
  while(iter_word != iter_end) {
    // If the beginning position is ahead of the end,
    // its a whitespace so add it to the output
    if(pos_word_1 < pos_word_0)
      output.push_back(arg_line.substr(pos_word_1, pos_word_0 - pos_word_1));
    
    // Find end of this word 
    pos_word_1 = iter_word->position() + iter_word->length();

    // If the quote is within this word...
    while(pos_word_0 <= pos_quote && pos_quote < pos_word_1 &&
          iter_quote != iter_end) {

      // Check if the quote is at the beginning or end of the word because
      // *technically* you could have a quote in the middle of a word (who would
      // do such a thing?!?!) in which case, we'll ignore
      if (pos_word_0 == pos_quote || pos_quote == pos_word_1 - 1) cnt_quote++;

      // Find the next quote position
      // (which may still be within the current word)
      iter_quote++;
      pos_quote = iter_quote->position() + iter_quote->length() - 1;
    }

    // Increment to the next word
    iter_word++;

    // If we are not within quotes, we are at the end of an argument
    if((cnt_quote % 2) < 1) {
      // Add the argument to the output
      output.push_back(arg_line.substr(pos_word_0, pos_word_1 - pos_word_0));

      // Find the start of the next word
      pos_word_0 = iter_word->position();
    }
  }

  return output;
}


/**@brief  Combines parsed arguments into a single string
 * 
 * @param  PARSED_ARGS: vector of strings
 *
 * @return string = PARSED_ARGS[0] + PARSED_ARGS[1] + ...
 */
wstring CollapseArguments (vector<wstring> parsed_args) {
  wstring output = L"";
  for (auto iter = parsed_args.begin(); iter != parsed_args.end(); ++iter) 
    output += *iter;
  return output; 
}


/**@brief  Collapse and re-parse a list of arguments
 * 
 * Needed for finding positional arguments after some parsing has been done
 * 
 * @param  PARSED_ARGS: vector of strings
 */
void ReparseArguments (vector<wstring> &parsed_args) {
  wstring arg_line = CollapseArguments(parsed_args);
  parsed_args = ParseArguments(arg_line);
}

  
/**@brief  Get and remove an argument at a certain position
 *
 * Finds the nth string from ARGS excluding whitespace elements. If the element
 * is found (i.e. within bounds), store as VALUE and removed from ARGS. If
 * subsequent whitespace exist, removed also. 
 *
 * @param  ARGS:    vector of strings from ParseArguments
 * @param  INDEX:   index of argument to get
 * @param  VALUE:   string at index
 *
 * @return TRUE if found
 */
bool GetArgument (vector<wstring> &args, const int index, wstring &value) {
  value.clear();
  if((index*2) < args.size()) {
    args[index*2].swap(value);
    if((index*2)+1 < args.size()) args[(index*2)+1].clear();
    return true;
  }
  return false;
}


/**@brief  Get and remove a flag matching a pattern
 *
 * Finds the string matching PATTERN, ignoring case, within ARGS. If the element
 * is found, removed from ARGS in addition to subsequent whitespace if exist.
 * 
 * @param  ARGS:    vector of strings from ParseArguments
 * @param  PATTERN: regex pattern to match
 *
 * @return TRUE if found
 */
bool GetArgument (vector<wstring> &args, wstring pattern) {
  // User-specified argument regex
  wregex arg_pattern(pattern, regex::icase);

  for (auto iter = args.begin(); iter != args.end(); ++iter) {
    if(regex_match(*iter, arg_pattern)) {
      (*iter).clear();                          // Clear the flag
      if(++iter != args.end()) (*iter).clear(); // Clear whitespace if needed
      return true;
    }
  }
   
  return false;
}


/**@brief  Get and remove a named argument matching a pattern
 *
 * Finds the string matching PATTERN, ignoring case, within ARGS. If the element
 * is found, the next argument is retrieved and stored as VALUE. The match,
 * value, whitespace separating the two are removed from ARGS in addition to
 * subsequent whitespace if exist. TRUE is only returned if the match is found
 * AND an argument follows.
 * 
 * @param  ARGS:    vector of strings from ParseArguments
 * @param  PATTERN: regex pattern to match
 * @param  VALUE:   argument string following match
 *
 * @return TRUE if pattern and value are found
 */
bool GetArgument (vector<wstring> &args, wstring pattern,wstring &value) {
  value.clear();

  // User-specified argument regex
  wregex arg_pattern(pattern, regex::icase);

  for (auto iter = args.begin(); iter != args.end(); ++iter) {
    if(regex_match(*iter, arg_pattern) && (iter+2) < args.end()) {
      (*iter).clear();                          // Clear the flag
      (*(++iter)).clear();                      // Clear the whitespace
      (*(++iter)).swap(value);                  // Get the value and clear
      if(++iter != args.end()) (*iter).clear(); // Clear whitespace if needed
      return true;
    }
  }
   
  return false;
}


// ------------------------------------------------------------------------- //
#endif  // GET_ARGUMENTS_H

