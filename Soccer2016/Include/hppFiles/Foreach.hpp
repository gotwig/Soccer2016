#ifndef SOCCER_FOREACH_HPP
#define SOCCER_FOREACH_HPP

// Preprocessor trick to allow nested loops
#define SOCCER_PP_CAT_IMPL(a, b) a ## b
#define SOCCER_PP_CAT(a, b) SOCCER_PP_CAT_IMPL(a, b)
#define SOCCER_ID(identifier) SOCCER_PP_CAT(auroraDetail_, identifier)
#define SOCCER_LINE_ID(identifier) SOCCER_PP_CAT(SOCCER_ID(identifier), __LINE__)


// Macro to emulate C++11 range-based for loop
// Instead of for (decl : range) you write FOREACH(decl, range) as in the following example
//
// std::vector<int> v = ...;
// FOREACH(int& i, v)
// {
//     i += 2;
// }
#define FOREACH(declaration, container)																											\
	if (bool SOCCER_LINE_ID(broken) = false) {} else																								\
	for (auto SOCCER_LINE_ID(itr) = (container).begin(); SOCCER_LINE_ID(itr) != (container).end() && !SOCCER_LINE_ID(broken); ++SOCCER_LINE_ID(itr))	\
	if (bool SOCCER_LINE_ID(passed) = false) {} else																								\
	if (SOCCER_LINE_ID(broken) = true, false) {} else																								\
	for (declaration = *SOCCER_LINE_ID(itr); !SOCCER_LINE_ID(passed); SOCCER_LINE_ID(passed) = true, SOCCER_LINE_ID(broken) = false)

#endif // SOCCER_FOREACH_HPP
