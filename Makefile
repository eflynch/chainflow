PROJECTS = chain.info chain.site

all:
	for project in $(PROJECTS) ; do \
		xcodebuild -project $$project/$$project.xcodeproj ; \
	done

