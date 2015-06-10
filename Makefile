PROJECTS = chain.info chain.site chain.test chain.device

all:
	for project in $(PROJECTS) ; do \
		xcodebuild -project $$project/$$project.xcodeproj ; \
	done

