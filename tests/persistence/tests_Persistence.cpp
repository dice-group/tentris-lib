#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <random>
#include <unistd.h>
#include <sys/wait.h>

TEST_CASE("NodeStorage persistence") {
	std::string const path = "/tmp/tentris-node-storage-persistence-test" + std::to_string(std::random_device{}());

	auto pid = fork();
	REQUIRE(pid >= 0);

	if (pid == 0) {
		execl("Persistence_phase1", "NPersistence_phase1", path.data(), nullptr);
		FAIL_CHECK("Expected successful exec");
	} else {
		int rc;
		int st = waitpid(pid, &rc, 0);
		REQUIRE(st >= 0);
		REQUIRE(WIFEXITED(rc));
	}

	pid = fork();
	REQUIRE(pid >= 0);

	if (pid == 0) {
		execl("Persistence_phase2", "Persistence_phase2", path.data(), nullptr);
		FAIL_CHECK("Expected successful exec");
	} else {
		int rc;
		int st = waitpid(pid, &rc, 0);
		REQUIRE(st >= 0);
		REQUIRE(WIFEXITED(rc));
	}
}
