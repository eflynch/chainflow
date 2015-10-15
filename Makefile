PROJECTS = chain.info chain.site chain.test chain.device chain.metric chain.zone chain.map chain.time chain.data

all:
	for project in $(PROJECTS) ; do \
		xcodebuild -project $$project/$$project.xcodeproj ; \
	done

clean:
	for project in $(PROJECTS) ; do \
		xcodebuild -project $$project/$$project.xcodeproj clean ; \
	done

