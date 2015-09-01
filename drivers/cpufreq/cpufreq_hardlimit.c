/*
 *  drivers/cpufreq/cpufreq_hardlimit.c
 *
 *  Copyright (C)  2013 Jean-Pierre Rasquin <yank555.lu@gmail.com>
 *            (C)  2014 LoungeKatt <twistedumbrella@gmail.com>
 *            (C)  2015 Placiano <placiano80@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/cpufreq_hardlimit.h>
#include <linux/cpufreq.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>

unsigned int hardlimit_max  = CPUFREQ_HARDLIMIT_MAX_STOCK;  		/* default to stock behaviour */
unsigned int hardlimit_min  = CPUFREQ_HARDLIMIT_MIN_STOCK;  		/* default to stock behaviour */
unsigned int current_limit_max        = CPUFREQ_HARDLIMIT_MAX_STOCK;
unsigned int current_limit_min        = CPUFREQ_HARDLIMIT_MIN_STOCK;
unsigned int userspace_dvfs_lock      = CPUFREQ_HARDLIMIT_USERSPACE_DVFS_ALLOW;	/* default allows userspace dvfs interaction */
unsigned int hardlimit_user_enforced   = HARDLIMIT_USER_DISABLED;


/* Externally reachable function */

/* Sanitize cpufreq to hardlimits */
unsigned int check_cpufreq_hardlimit(unsigned int freq)
{
// Called way too often, even when debugging
//	#ifdef CPUFREQ_HARDLIMIT_DEBUG
//	pr_info("[HARDLIMIT] check_cpufreq_hardlimit : min = %u / max = %u / freq = %u / result = %u \n",
//			current_limit_min,
//			current_limit_max,
//			freq,
//			max(current_limit_min, min(current_limit_max, freq))
//		);
//	#endif
	return max(current_limit_min, min(current_limit_max, freq));
}

/* Update limits in cpufreq */
void reapply_hard_limits(void)
{
	#ifdef CPUFREQ_HARDLIMIT_DEBUG
	pr_info("[HARDLIMIT] reapply_hard_limits - before : min = %u / max = %u \n",
			current_limit_min,
			current_limit_max
		);
	#endif

	/* Recalculate the currently applicable min/max */
		current_limit_min  = hardlimit_min;
		current_limit_max  = hardlimit_max;

	#ifdef CPUFREQ_HARDLIMIT_DEBUG
	pr_info("[HARDLIMIT] reapply_hard_limits - after : min = %u / max = %u \n",
			current_limit_min,
			current_limit_max
		);
	#endif
	update_scaling_limits(current_limit_min, current_limit_max);
}

/* User enable/disable */
unsigned int hardlimit_user_enforced_status(void)
{
    return hardlimit_user_enforced;
}

/* Scaling min/max lock */
unsigned int userspace_dvfs_lock_status(void)
{
	return userspace_dvfs_lock;
}

/* ------------------------------------------------------------------------------ */
/* sysfs interface functions                                                      */
/* ------------------------------------------------------------------------------ */

/* sysfs interface for "hardlimit_max_screen_on" */
static ssize_t hardlimit_max_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", hardlimit_max);
}

static ssize_t hardlimit_max_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	unsigned int new_hardlimit, i;

	struct cpufreq_frequency_table *table;

	if (!sscanf(buf, "%du", &new_hardlimit))
		return -EINVAL;

	if (new_hardlimit == hardlimit_max)
		return count;

	table = cpufreq_frequency_get_table(0); /* Get frequency table */

	for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
		if (table[i].frequency == new_hardlimit) {
			hardlimit_max = new_hardlimit;
			reapply_hard_limits();
			return count;
		}

	return -EINVAL;

}

/* sysfs interface for "hardlimit_min_screen_on" */
static ssize_t hardlimit_min_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", hardlimit_min);
}

static ssize_t hardlimit_min_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	unsigned int new_hardlimit, i;

	struct cpufreq_frequency_table *table;

	if (!sscanf(buf, "%du", &new_hardlimit))
		return -EINVAL;

	if (new_hardlimit == hardlimit_min)
		return count;

	table = cpufreq_frequency_get_table(0); /* Get frequency table */

	for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
		if (table[i].frequency == new_hardlimit) {
			hardlimit_min = new_hardlimit;
			reapply_hard_limits();
			return count;
		}

	return -EINVAL;

}

/* sysfs interface for "hardlimit_user_enforced" */
static ssize_t hardlimit_user_enforced_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", hardlimit_user_enforced);
}

static ssize_t hardlimit_user_enforced_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    
    unsigned int new_hardlimit_user_enforced;
    
    if (!sscanf(buf, "%du", &new_hardlimit_user_enforced))
        return -EINVAL;
    
    if (new_hardlimit_user_enforced == hardlimit_user_enforced)
        return count;
    
    if (new_hardlimit_user_enforced == HARDLIMIT_USER_DISABLED
        || new_hardlimit_user_enforced == HARDLIMIT_USER_ENFORCED) {
        hardlimit_user_enforced = new_hardlimit_user_enforced;
        return count;
    }
    
    /* We should never get here */
    return -EINVAL;
    
}

/* sysfs interface for "userspace_dvfs_lock" */
static ssize_t userspace_dvfs_lock_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", userspace_dvfs_lock);
}

static ssize_t userspace_dvfs_lock_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	unsigned int new_userspace_dvfs_lock;

	if (!sscanf(buf, "%du", &new_userspace_dvfs_lock))
		return -EINVAL;

	if (new_userspace_dvfs_lock == userspace_dvfs_lock)
		return count;

    if (new_userspace_dvfs_lock == CPUFREQ_HARDLIMIT_USERSPACE_DVFS_ALLOW
        || new_userspace_dvfs_lock == CPUFREQ_HARDLIMIT_USERSPACE_DVFS_IGNORE
        || new_userspace_dvfs_lock == CPUFREQ_HARDLIMIT_USERSPACE_DVFS_REFUSE) {
        userspace_dvfs_lock = new_userspace_dvfs_lock;
        return count;
    } else {
        return -EINVAL;
    }

	/* We should never get here */
	return -EINVAL;

}

/* sysfs interface for "available_frequencies" */
static ssize_t available_frequencies_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	unsigned int i;
	ssize_t j = 0;

	struct cpufreq_frequency_table *table;

	table = cpufreq_frequency_get_table(0); /* Get frequency table */

	for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
		j += sprintf(&buf[j], "%d ", table[i].frequency);

	j += sprintf(&buf[j], "\n");
	return j;
}

/* sysfs interface for "current_limit_min" */
static ssize_t current_limit_min_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", current_limit_min);
}

/* sysfs interface for "current_limit_max" */
static ssize_t current_limit_max_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", current_limit_max);
}

/* sysfs interface for "version" */
static ssize_t version_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", CPUFREQ_HARDLIMIT_VERSION);
}

/* ------------------------------------------------------------------------------ */
/* sysfs interface functions - end                                                */
/* ------------------------------------------------------------------------------ */

#ifdef CPUFREQ_HARDLIMIT_LEGACY_INTERFACE
/* ------------------------------------------------------------------------------ */
/* sysfs legacy interface structure                                               */
/* ------------------------------------------------------------------------------ */

static struct kobject *legacy_hardlimit_kobj;

/* Define sysfs entry attributes */

static struct kobj_attribute legacy_hardlimit_max_attribute =
__ATTR(hardlimit, 0666, hardlimit_max_show, hardlimit_max_store);

static struct kobj_attribute legacy_available_frequencies_attribute =
__ATTR(available_frequencies, 0444, available_frequencies_show, NULL);

static struct kobj_attribute legacy_current_limit_min_attribute =
__ATTR(current_limit_min, 0444, current_limit_min_show, NULL);

static struct kobj_attribute legacy_current_limit_max_attribute =
__ATTR(current_limit_max, 0444, current_limit_max_show, NULL);

static struct kobj_attribute legacy_version_attribute =
__ATTR(version, 0444, version_show, NULL);

static struct attribute *legacy_hardlimit_attrs[] = {
	&legacy_hardlimit_max_attribute.attr,
	&legacy_available_frequencies_attribute.attr,
	&legacy_current_limit_min_attribute.attr,
	&legacy_current_limit_max_attribute.attr,
	&legacy_version_attribute.attr,
	NULL,
};

static struct attribute_group legacy_hardlimit_attr_group = {
.attrs = legacy_hardlimit_attrs,
};

#endif
/* ------------------------------------------------------------------------------ */
/* sysfs legacy interface structure - end                                         */
/* ------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------ */
/* sysfs new interface structure                                                  */
/* ------------------------------------------------------------------------------ */

static struct kobject *hardlimit_kobj;

/* Define sysfs entry attributes */

static struct kobj_attribute hardlimit_max_attribute =
__ATTR(scaling_max_freq, 0666, hardlimit_max_show, hardlimit_max_store);

static struct kobj_attribute hardlimit_min_attribute =
__ATTR(scaling_min_freq, 0666, hardlimit_min_show, hardlimit_min_store);

static struct kobj_attribute hardlimit_user_enforced_attribute =
__ATTR(hardlimit_user_enforced, 0666, hardlimit_user_enforced_show, hardlimit_user_enforced_store);

static struct kobj_attribute userspace_dvfs_lock_attribute =
__ATTR(userspace_dvfs_lock, 0666, userspace_dvfs_lock_show, userspace_dvfs_lock_store);

static struct kobj_attribute available_frequencies_attribute =
__ATTR(available_frequencies, 0444, available_frequencies_show, NULL);

static struct kobj_attribute current_limit_min_attribute =
__ATTR(current_limit_min, 0444, current_limit_min_show, NULL);

static struct kobj_attribute current_limit_max_attribute =
__ATTR(current_limit_max, 0444, current_limit_max_show, NULL);

static struct kobj_attribute version_attribute =
__ATTR(version, 0444, version_show, NULL);

static struct attribute *hardlimit_attrs[] = {
	&hardlimit_max_attribute.attr,
	&hardlimit_min_attribute.attr,
    &hardlimit_user_enforced_attribute.attr,
	&userspace_dvfs_lock_attribute.attr,
	&available_frequencies_attribute.attr,
	&current_limit_min_attribute.attr,
	&current_limit_max_attribute.attr,
	&version_attribute.attr,
	NULL,
};

static struct attribute_group hardlimit_attr_group = {
.attrs = hardlimit_attrs,
};

/* ------------------------------------------------------------------------------ */
/* sysfs new interface structure - end                                            */
/* ------------------------------------------------------------------------------ */

int hardlimit_init(void)
{
#ifdef CPUFREQ_HARDLIMIT_LEGACY_INTERFACE
	int legacy_hardlimit_retval;
#endif
	int hardlimit_retval;

	/* Enable the legacy sysfs interface */
#ifdef CPUFREQ_HARDLIMIT_LEGACY_INTERFACE
        legacy_hardlimit_kobj = kobject_create_and_add("cpufreq", kernel_kobj);
        if (!legacy_hardlimit_kobj) {
                return -ENOMEM;
        }
        legacy_hardlimit_retval = sysfs_create_group(legacy_hardlimit_kobj, &legacy_hardlimit_attr_group);
        if (legacy_hardlimit_retval)
                kobject_put(legacy_hardlimit_kobj);
#endif

	/* Enable the new sysfs interface */

        hardlimit_kobj = kobject_create_and_add("cpufreq_hardlimit", kernel_kobj);
        if (!hardlimit_kobj) {
                return -ENOMEM;
        }
        hardlimit_retval = sysfs_create_group(hardlimit_kobj, &hardlimit_attr_group);
        if (hardlimit_retval)
                kobject_put(hardlimit_kobj);

#ifdef CPUFREQ_HARDLIMIT_LEGACY_INTERFACE
        if (!hardlimit_retval && !legacy_hardlimit_retval) {
#else
        if (!hardlimit_retval) {
#endif
        }

        return (hardlimit_retval);
}
/* end sysfs interface */

void hardlimit_exit(void)
{
	kobject_put(hardlimit_kobj);
}

module_init(hardlimit_init);
module_exit(hardlimit_exit);
