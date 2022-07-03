package main

import (
	"encoding/json"
	"fmt"
	"sort"
	"strconv"
	"strings"

	"github.com/hyperledger/fabric-contract-api-go/contractapi"
)

// SmartContract provides functions for managing a car
type SmartContract struct {
	contractapi.Contract
}

// CampItem describes basic details of the Candidate
type CampItem struct {
	Name    string `json:"name"`
	Votes   string `json:"votes"`
	Ranking string `json:"ranking"`
}

// QueryResult structure used for handling result of query
type QueryResult struct {
	campitem CampItem
}

// InitLedger adds a base set of cars to the ledger
func (s *SmartContract) InitLedger(ctx contractapi.TransactionContextInterface) error {
	campitems := []CampItem{
		CampItem{Name: "Trump", Votes: "0", Ranking: "1"},
		CampItem{Name: "Byden", Votes: "0", Ranking: "1"},
		CampItem{Name: "Obama", Votes: "0", Ranking: "1"},
		CampItem{Name: "FlappyBear", Votes: "0", Ranking: "1"},
		CampItem{Name: "pty", Votes: "0", Ranking: "1"},
	}
	for i := range campitems {
		campitemAsBytes, _ := json.Marshal(campitems[i])
		err := ctx.GetStub().PutState(campitems[i].Name, campitemAsBytes)

		if err != nil {
			return fmt.Errorf("Failed to put to world state. %s", err.Error())
		}
	}

	return nil
}

// QueryCandidate returns the candidate stored in the world state with given name
func (s *SmartContract) QueryCandidate(ctx contractapi.TransactionContextInterface, CandName string) (*CampItem, error) {
	campitemAsBytes, err := ctx.GetStub().GetState(CandName)

	if err != nil {
		return nil, fmt.Errorf("Failed to read from world state. %s", err.Error())
	}

	if campitemAsBytes == nil {
		return nil, fmt.Errorf("%s does not exist", CandName)
	}

	campitem := new(CampItem)
	_ = json.Unmarshal(campitemAsBytes, campitem)

	return campitem, nil
}

// VoteCand add a vote to the specific Candidate
func (s *SmartContract) VoteCand(ctx contractapi.TransactionContextInterface, CandName string) error {
	campitem, err := s.QueryCandidate(ctx, CandName)

	if err != nil {
		return err
	}

	if campitem == nil {
		return fmt.Errorf("The Candidate doesn't exist!\n")
	}

	NewVotes, err := strconv.Atoi(campitem.Votes)
	campitem.Votes = strconv.Itoa(NewVotes + 1)

	campitemAsBytes, _ := json.Marshal(campitem)

	return ctx.GetStub().PutState(campitem.Name, campitemAsBytes)
}

// JoinCamp adds a new candidate to the world state with given details
func (s *SmartContract) JoinCamp(ctx contractapi.TransactionContextInterface, CandName string) error {
	campitem := CampItem{
		Name:    CandName,
		Votes:   "0",
		Ranking: "1",
	}

	isCand, _ := s.QueryCandidate(ctx, CandName)

	if isCand != nil {
		return fmt.Errorf("%s has already joined the campaign", CandName)
	}

	campitemAsBytes, _ := json.Marshal(campitem)

	return ctx.GetStub().PutState(CandName, campitemAsBytes)
}

// QueryAllCandidates returns all candidates found in world state
func (s *SmartContract) QueryAllCandidates(ctx contractapi.TransactionContextInterface) ([]QueryResult, error) {

	results, _ := s.SortCamp(ctx)

	Ranklist := []QueryResult{}

	LastVote := -1
	RankNow := 0

	// CampPoint := new(QueryResult)
	for i := range results {
		NowVote, _ := strconv.Atoi(results[i].campitem.Votes)
		if NowVote > LastVote {
			RankNow = RankNow + 1
			results[i].campitem.Ranking = strconv.Itoa(RankNow)
		} else {
			results[i].campitem.Ranking = strconv.Itoa(RankNow)
		}

		LastVote = NowVote
		Ranklist = append(Ranklist, *results[i])

		campitemAsBytes, _ := json.Marshal(results[i].campitem)
		ctx.GetStub().PutState(results[i].campitem.Name, campitemAsBytes)
	}

	return Ranklist, nil
}

type SortQue []*QueryResult

func (s SortQue) Len() int {
	return len(s)
}

func (s SortQue) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

func (s SortQue) Less(i, j int) bool {
	value := strings.Compare(s[i].campitem.Votes, s[j].campitem.Votes)
	if value == 0 {
		return strings.Compare(s[i].campitem.Name, s[j].campitem.Name) == 1
	} else {
		return value > 0
	}
}

func (s *SmartContract) SortCamp(ctx contractapi.TransactionContextInterface) ([]*QueryResult, error) {
	startKey := ""
	endKey := ""

	resultsIterator, err := ctx.GetStub().GetStateByRange(startKey, endKey)

	if err != nil {
		return nil, nil
	}
	defer resultsIterator.Close()

	results := []*QueryResult{}

	for resultsIterator.HasNext() {
		queryResponse, err := resultsIterator.Next()

		if err != nil {
			return nil, nil
		}

		campitem := new(CampItem)
		_ = json.Unmarshal(queryResponse.Value, campitem)

		queryResult := QueryResult{campitem: *campitem}
		results = append(results, &queryResult)

	}
	sort.Sort(SortQue(results))

	return results, nil
}

func main() {

	chaincode, err := contractapi.NewChaincode(new(SmartContract))

	if err != nil {
		fmt.Printf("Error create fabvote chaincode: %s", err.Error())
		return
	}

	if err := chaincode.Start(); err != nil {
		fmt.Printf("Error starting fabcar chaincode: %s", err.Error())
	}
}
