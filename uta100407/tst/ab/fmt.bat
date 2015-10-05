indent  -kr -nut -ts4 -l80 -bad -bap -br -brf -brs -ce -cli2 -nsaw -nsai -nprs -sob %1
:indent  -bad -bap -br -brf -brs -cdw -ce -cli2 -fca -i4 -l80 -nbc -nbfda -ncs -npcs -nprs -nsaf -nsai -nsaw -nut -ts4 %1.c -o %1.fmt
:indent -kr -nbad -nbap -bbo -bc -br -brs -c33 -cd33 -cdb -ce -ci4 -cli0 -cp33 -di16 -fc1 -fca -hnl -i4 -ip4 -l75 -lp -npcs -nprs -psl -saf -sai -saw -sc -nsob -nss -ts8 %1 -o %2
     

:-bad :--blank-lines-after-declarations :Force blank lines after the declarations
:-bap :--blank-lines-after-procedures : Force blank lines after procedure bodies.
:-br  :--braces-on-if-line Put braces on line with if, etc.
:-brs :--braces-on-struct-decl-line Put braces on struct declaration line.
:-brf :  to place function opening brace after function declaration.
:-cdw :--cuddle-do-while Cuddle while of do {} while; and preceeding }.
:-ce  :--cuddle-else Cuddle else and preceeding }.
:-clin:--case-indentationn Case label indent of n spaces.
:-fca :--format-all-comments Do not disable all formatting of comments.
:-in  :--indent-leveln Set indentation level to n spaces.
:-ln  :--line-lengthn Set maximum line length for non-comment lines to n.
:-nbc :--no-blank-lines-after-commas Do not force newlines after commas in declarations.
:-nbfda:--dont-break-function-decl-args Don't put each argument in a function declaration on a seperate line.
:-ncs :--no-space-after-casts Do not put a space after cast operators.
:-npcs:--no-space-after-function-call-names Do not put space after the function in function calls.
:-nprs:--no-space-after-parentheses Do not put a space after every '(' and before every ')'.
:-nsaf:--no-space-after-for Do not put a space after every for.
:-nsai:--no-space-after-if Do not put a space after every if.
:-nsaw:--no-space-after-while Do not put a space after every while.
:-nut :--no-tabs Use spaces instead of tabs.
:-tsn :--tab-sizen Set tab size to n spaces.

:---

:-bbb :--blank-lines-before-block-comments Force blank lines before block comments.
:-bbo :--break-before-boolean-operator Prefer to break long lines before boolean operators.
:-bc  :--blank-lines-after-commas Force newline after comma in declaration.
:-bl  :--braces-after-if-line Put braces on line after if, etc.
:-blin:--brace-indentn Indent braces n spaces.
:-bls :--braces-after-struct-decl-line Put braces on the line after struct declaration lines.
:-bs  :--Bill-Shannon --blank-before-sizeof Put a space between sizeof and its argument.
:-cn  :--comment-indentationn Put comments to the right of code in column n.
:-cbin:--case-brace-indentationn Indent braces after a case label N spaces.
:-cdn :--declaration-comment-columnn Put comments to the right of the declarations in column n.
:-cdb :--comment-delimiters-on-blank-lines Put comment delimiters on blank lines.
:-cin :--continuation-indentationn Continuation indent of n spaces.
:-cpn :--else-endif-columnn Put comments to the right of #else and #endif statements in column n.
:-cs  :--space-after-cast Put a space after a cast operator.
:-dn  :--line-comments-indentationn Set indentation of comments not to the right of code to n spaces.
:-bfda:--break-function-decl-args Break the line before all arguments in a declaration.
:-bfde:--break-function-decl-args Break the line after the last argument in a declaration.
:-din :--declaration-indentationn Put variables in column n.
:-fc1 :--format-first-column-comments Format comments in the first column.
:-gnu :--gnu-style Use GNU coding style. This is the default.
:-hnl :--honour-newlines Prefer to break long lines at the position of newlines in the input.
:-ipn :--parameter-indentationn Indent parameter types in old-style function definitions by n spaces.
:-kr  :--k-and-r-style Use Kernighan & Ritchie coding style.
:-lcn :--comment-line-lengthn Set maximum line length for comment formatting to n.
:-lp  :--continue-at-parentheses Line up continued lines at parentheses.
:-lps :--leave-preprocessor-space Leave space between # and preprocessor directive.
:-nbad:--no-blank-lines-after-declarations Do not force blank lines after declarations.
:-nbap:--no-blank-lines-after-procedures Do not force blank lines after procedure bodies.
:-nbbo:--break-after-boolean-operator Do not prefer to break long lines before boolean operators.
:-ncdb:--no-comment-delimiters-on-blank-lines Do not put comment delimiters on blank lines.
:-ncdw:--dont-cuddle-do-while Do not cuddle } and the while of a do {} while;.
:-nce :--dont-cuddle-else Do not cuddle } and else.
:-nfc1:--dont-format-first-column-comments Do not format comments in the first column as normal.
:-nfca:--dont-format-comments Do not format any comments.
:-nhnl:--ignore-newlines Do not prefer to break long lines at the position of newlines in the input.
:-nip :--no-parameter-indentation Zero width indentation for parameters.
:-nlp :--dont-line-up-parentheses Do not line up parentheses.
:-npsl:--dont-break-procedure-type Put the type of a procedure on the same line as its name.
:-nsc :--dont-star-comments Do not put the * character at the left of comments.
:-nsob:--leave-optional-blank-lines Do not swallow optional blank lines.
:-nss :--dont-space-special-semicolon Do not force a space before the semicolon after certain statements. Disables -ss.
:-nv  :--no-verbosity Disable verbose mode.
:-orig:--original Use the original Berkeley coding style.
:-npro:--ignore-profile Do not read .indent.pro files.
:-pcs :--space-after-procedure-calls Insert a space between the name of the procedure being called and the (.
:-pin :--paren-indentationn Specify the extra indentation per open parentheses '(' when a statement is broken. See Statements. 
:-pmt :--preserve-mtime Preserve access and modification times on output files.
:-prs :--space-after-parentheses Put a space after every '(' and before every ')'.
:-psl :--procnames-start-lines Put the type of a procedure on the line before its name.
:-saf :--space-after-for Put a space after each for.
:-sai :--space-after-if Put a space after each if.
:-saw :--space-after-while Put a space after each while.
:-sbin:--struct-brace-indentationn Indent braces of a struct, union or enum N spaces.
:-sc  :--start-left-side-of-comments Put the * character at the left of comments.
:-sob :--swallow-optional-blank-lines Swallow optional blank lines.
:-ss  :--space-special-semicolon On one-line for and while statments, force a blank before the semicolon.
:-st  :--standard-output Write to standard output.
:-T   :Tell indent the name of typenames.
:-ut  :--use-tabs Use tabs. This is the default.
:-v   :--verbose Enable verbose mode.
:-version :Output the version number of indent.
