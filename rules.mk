#
# ����c�ļ����ɶ�Ӧ�������ļ�
#
# 
   
$(obj)%.d: %.c
	@echo "create depend"
	@echo $*
	$(CC) -M $(CCFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(obj)\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm $@.$$$$    
	

