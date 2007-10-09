#ifndef __CONF_H__
#define __CONF_H__

/*configuration*/
/* the simple query <SectionName>(<argument_of_the_section>).<variable_name>
 * Example: General(redleaf).port
 */
char *get_configuration_value(const char *expr);

/*----init-------*/
/*init default general values*/

/*----parser-----*/
/*read syntax and context tree*/
//int read_syn_tree(char *buffer,int size);
void load_configuration(char *buffer,int size);

#endif
