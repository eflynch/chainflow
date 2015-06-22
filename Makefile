PROJECTS = chain.info chain.site chain.test chain.device chain.metric

all:
	for project in $(PROJECTS) ; do \
		xcodebuild -project $$project/$$project.xcodeproj ; \
	done

