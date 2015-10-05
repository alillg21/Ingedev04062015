/** \file
 * Unitary test cases for UTA components
 *
 * - tcutanot.h: non-UTA functions
 * - tcutasys.h: SYS group of components (system functions)
 * - tcutapri.h: PRI group of components (primitive functions)
 * - tcutafnc.h: various functional tests
 */
 
#ifndef __TST_H

/** \defgroup tstgroup TST Unitary test cases
 *
 * The unitary test cases are declared in the header file tst.h in tst directory.
 * They are three types of unitary test cases:
 *  - NON UTA test cases are made as a preliminary step before implementing a feature in UTA
 *    They can be used later when a dirver or OS version change.
 *  - SYS test cases are to demonstrate the behaviour of a function in a SYS component of UTA
 *  - PRI test cases are designed for functions implemented in primitive components
 *
 * The unitary test cases serve for several purposes:
 *  - during a feature design the test case is used to test the implementation
 *  - it demonstrates the usage of the functions
 *  - it tests the function behaviour in exceptional situations
 *  - it is used for non-regression testing
 *
 * The unitary test cases are almost all independent:
 * there is no preconditions before test execution, and the test case should not modify the state of the terminal.
 * Each test case is a function witho no parameters and no return value.
 *
 * There are some exceptions of this rule: the EMV test case and some of SSA test cases are interdependent.
 *
 * The naming convention for UTA test cases is: tcDDNNNN, whetre
 *  - tc is fixed and means "test case"
 *  - DD are the initials of the developer's First and Last name
 *  - NNNN is the unique number of the test case
 *
 * Each developer has his/her own subdirectory in uta\\tst.
 * The responsibility on the test case quality and updates is on its owner.
 *
 * @{
*/

/** \defgroup tcnongroup TCNON Non UTA unitary test cases
 * @{
*/
#include "tcnonuta.h"
/** @} */

/** \defgroup tcsysgroup TCSYS Unitary test cases for system components
 * @{
*/
#include "tcutasys.h"
/** @} */

/** \defgroup tcprigroup TCPRI Unitary test cases for primitive components
 * @{
*/
#include "tcutapri.h"
/** @} */

/** \defgroup tcfncgroup TCFNC Functional test cases for various projects
 * @{
*/
#include "tcutafnc.h"
/** @} */

/** \weakgroup tctodogroup TCTODO Unitary test cases not classified yet (under construction)
 * @}
*/


#define __TST_H
#endif

