#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__



typedef struct {
  char *fw_name;
  char *fw_date;
  char *fw_sn;
  char *fw_version;
  char *fw_compile_data;
  char *fw_compile_time;
} fw_tag_t;

extern fw_tag_t *tag;



void SystemClock_Config(void);


#endif /* __SYSTEM_CONFIG_H__ */
