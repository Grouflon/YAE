# TODO

## Philosophy
- String, Arrays, whatevs shoud not transmit their allocators. Nothing should have a null allocator. Transformation functions such as string.h functions should always use the scrath allocator. Allocators should be easier to init and use (stop juggling with pointers and ref)

## Hot-Reload
- Mirror: Step away from static initilization even more and make it so that we can refresh the types definition/population at runtime (clear content and run through the new populate code). Since all is data I guess it should be okay (if pointer functions works ? or maybe it can go through the virtual type wrapper, I don't know) 

## Debugging
- Test if we can load natvis file from the project
- Plug back RemedyBG on hot-restart

## Cleaning and coding rules
- Rename Program as Core. Maybe
- Cleanup lowercase/uppercase file names. Decide of some rule

## Sublime
- insert date time with plugins snippets and macros https://forum.sublimetext.com/t/easiest-way-to-insert-date-time-with-a-single-keypress/4134/10