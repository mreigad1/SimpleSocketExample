NEW_USER=${1}
USER_FILE=${2}
OVERRIDE=${3}
if [ ! -z "${NEW_USER}" ]; then
	if [ ! -f ${USER_FILE} ] || [ "${OVERRIDE}" = "override" ]; then
		echo ${NEW_USER} > ${USER_FILE}
	fi
fi