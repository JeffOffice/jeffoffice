# Notas, comentários e ligações

<!-- keywords: painel de comentários, novo comentário, responder, resolver, reabrir, conversa, comentário anterior, comentário seguinte, mostrar comentários, converter notas em comentários, converter comentários em notas, ortografia, verificar ortografia, palavra errada, F7, ignorar tudo, alterar tudo, adicionar ao dicionário, dicionário de sinónimos, sinónimos, lista de palavras, nota, notas, comentário, comentários, passar o rato, marca vermelha, canto, nova nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas as notas, ligação, ligações, hiperligação, endereço, página web, correio eletrónico, lugar neste livro, Ctrl+K, Ctrl+clique, sugestão de ecrã, abrir ligação, remover ligação, conversa, estatísticas do livro de trabalho, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células unidas, ficheiros recentes -->

## Notas

Uma **nota** é um pequeno texto associado a uma célula. Uma célula que tenha uma nota mostra uma pequena marca vermelha no canto superior direito. Quando coloca o ponteiro sobre a célula, a nota aparece ao lado dela; quando afasta o ponteiro, a nota desaparece.

Para colocar uma nota numa célula, selecione-a e utilize **Revisão > Nova Nota** (ou prima **Shift+F2**). Escreva o texto e clique em **OK**. Numa célula que já tenha uma nota, o mesmo comando aparece como **Editar Nota**. Se apagar o texto, a nota é removida.

- **Eliminar Nota** remove as notas das células selecionadas.
- **Mostrar ou Ocultar Nota** mantém a nota das células selecionadas sempre visível na folha, numa caixa ao lado de cada célula, ou volta a ocultá-la.
- **Mostrar Todas as Notas** faz o mesmo para todas as notas da folha; o mesmo botão passa então a ler-se **Ocultar Todas as Notas**.
- **Nota Anterior** e **Nota Seguinte** deslocam-se de uma célula com nota para a seguinte.

Cada uma destas ações pode ser anulada com um único passo. Uma nota acompanha a sua célula quando linhas ou colunas são inseridas ou eliminadas, e quando as células são ordenadas. **Limpar Tudo** remove as notas e os comentários das células que limpa.

Uma célula pode também conter uma conversa de **comentários**, com respostas. Veja a próxima secção.

As notas podem ser guardadas num livro .xlsx. Um livro aberto a partir de um tipo de ficheiro mais antigo, ou de um ficheiro de texto, só mantém as notas depois de o guardar como livro .xlsx.

## Comentários

Um **comentário** é semelhante a uma nota, mas inicia uma **conversa**: as pessoas podem responder-lhe e uma delas pode marcar a conversa como **resolvida** quando a questão estiver respondida. Cada comentário mostra quem o escreveu e quando. Uma célula com uma conversa tem a mesma marca vermelha de uma célula com nota. Coloque o ponteiro sobre a célula para ler a conversa completa, com cada resposta recuada por baixo do comentário a que responde. Uma conversa resolvida começa com a palavra **Resolvido**.

No separador **Revisão**, no grupo **Comentários**:

- **Novo Comentário** (**Ctrl+Shift+F2**) inicia uma conversa na célula selecionada. Escreva o texto e clique em **OK**. Funciona numa célula que não tenha nota nem comentários.
- **Responder** acrescenta o seu texto à conversa da célula selecionada.
- **Resolver Conversa** marca a conversa como concluída. O mesmo botão passa então a ler-se **Reabrir Conversa**, que a volta a abrir.
- **Eliminar Comentário** remove as conversas das células selecionadas.
- **Comentário Anterior** e **Comentário Seguinte** deslocam-se de uma célula com conversa para a seguinte.
- **Mostrar Comentários** abre o painel **Comentários** ao lado da folha. Lista todas as conversas da folha como cartões: a célula, cada comentário com o respetivo autor e hora, e os botões **Responder**, **Resolver** ou **Reabrir**, e **Eliminar**. **Ir Para** move para a célula. O painel acompanha todas as alterações, incluindo **Anular**.
- **Converter Notas em Comentários** transforma todas as notas simples da folha numa conversa de um comentário. **Converter Comentários em Notas** transforma cada conversa de novo numa nota. Uma nota não tem respostas, por isso os comentários de uma conversa passam a ser uma única nota com todo o texto; o JeffCrunch pede confirmação antes de o fazer numa conversa que tenha respostas.

Cada uma destas ações pode ser anulada com um único passo.

Os programas que não conseguem mostrar conversas apresentam, em seu lugar, uma nota simples. O JeffCrunch mantém essa nota, com o texto da conversa, sempre que guarda. As conversas, as respostas e as pessoas que as escreveram mantêm-se como estavam quando guarda e volta a abrir o livro. Uma conversa com mais de 500 comentários é apresentada e mantida, mas o JeffCrunch não lhe acrescenta nada nem a altera.

Os comentários são guardados num livro .xlsx. Um livro aberto a partir de um tipo de ficheiro mais antigo, ou de um ficheiro de texto, só mantém os comentários depois de o guardar como livro .xlsx.

## Ligações

Uma **ligação** leva-o a outro lugar quando a segue: para uma página web, para um ficheiro, para um endereço de correio eletrónico ou para um lugar neste livro (uma célula, células ou um nome).

Para colocar uma ligação numa célula, selecione-a e utilize **Inserir > Ligação** (ou prima **Ctrl+K**). Escolha para onde deve ir, escreva o endereço ou o lugar (ou escolha um dos ficheiros abertos recentemente em **Ou um ficheiro recente**) e, se quiser, o texto que a célula deve mostrar e uma **sugestão de ecrã**. Numa célula que já tenha uma ligação, o mesmo comando aparece como **Editar Ligação**. **Remover Ligação** retira as ligações das células selecionadas.

Coloque o ponteiro sobre uma célula com ligação para ver para onde leva. Para a seguir, mantenha **Ctrl** premido e clique na célula, ou selecione a célula e utilize **Abrir Ligação** no menu do botão direito. Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por iniciativa própria. Uma página web ou um endereço de correio eletrónico abre-se no seu programa habitual quando segue a ligação. Uma ligação para um ficheiro, ou para outro tipo de endereço, pede-lhe confirmação primeiro. Uma ligação que executasse um script nunca é aberta.

Uma ligação acompanha a sua célula quando linhas ou colunas são inseridas ou eliminadas, e quando as células são ordenadas, e é guardada com o livro.

## Estatísticas do Livro de Trabalho

**Estatísticas do Livro de Trabalho** (separador **Revisão**) conta o que o livro contém: quantas **Folhas** tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. As folhas que eliminou não são contadas. Os mesmos números estão na página **Informações** do separador Ficheiro.

## Ortografia

**Ortografia** (separador **Revisão**, ou **F7**) verifica as palavras das células de texto, uma palavra errada de cada vez. Se tiver selecionado mais do que uma célula, verifica apenas essas células. Caso contrário, verifica a folha inteira. Não analisa fórmulas, números nem datas.

A janela mostra a palavra que não consta do dicionário, uma caixa chamada **Alterar para** com a melhor sugestão e uma lista de outras sugestões. Escolha uma sugestão ou escreva a sua própria palavra e, depois, escolha:

- **Alterar** substitui a palavra errada, nessa célula, pela nova palavra. Cada alteração pode ser anulada com um único passo.
- **Alterar Tudo** substitui a mesma palavra em todas as células verificadas, como um único passo que pode ser anulado.
- **Ignorar** deixa a palavra como está desta vez. **Ignorar Tudo** deixa-a como está em todo o lado até fechar o JeffCrunch.
- **Adicionar** acrescenta a palavra ao seu dicionário pessoal, pelo que deixa de ser assinalada. O seu dicionário é partilhado com as outras aplicações do JeffOffice.

**Ficheiro > Opções > Verificação** define se as palavras em maiúsculas e as palavras com números são verificadas. O JeffCrunch verifica no idioma em que é apresentado, se tiver um dicionário para esse idioma, e em inglês dos Estados Unidos caso contrário.

Enquanto escreve numa célula, uma palavra que não conste do dicionário fica com uma linha ondulada vermelha por baixo. Clique com o botão direito na palavra para escolher uma correção, ou para escolher **Ignorar Tudo** ou **Adicionar ao Dicionário**. As células da grelha não são sublinhadas, e uma entrada que comece com um sinal de igual nunca é verificada.

## Dicionário de sinónimos

**Dicionário de Sinónimos** (separador **Revisão**, ou **Shift+F7**) encontra palavras com um significado quase igual ao de uma palavra numa célula. Procura a primeira palavra do texto da célula em que está. Escreva outra palavra em **Procurar** e prima **Enter** para procurar essa. Escolha uma palavra na lista e clique em **Substituir**, ou faça duplo clique nela. A palavra na célula é substituída, e uma primeira letra maiúscula mantém-se maiúscula. **Procurar** procura a palavra que escolheu, para seguir uma cadeia de significados. A lista de palavras funciona em inglês.

## Verificar Acessibilidade

**Verificar Acessibilidade** (separador **Revisão**) abre uma pequena janela que lista o que, no livro, é difícil de usar para alguns leitores, ou para um leitor de ecrã (um programa que lê o ecrã em voz alta). Cada linha começa por **Erro**, **Aviso** ou **Sugestão**. Clique numa linha para ir para a célula.

Procura:

- Um gráfico ou uma imagem sem **texto alternativo** (as palavras que descrevem o que é mostrado). Isto é um erro.
- Uma tabela sem linha de cabeçalho, pelo que as suas colunas não têm nomes. Isto é um erro.
- **Células unidas**, que são difíceis de percorrer com o teclado. Isto é um aviso.
- Texto difícil de ler em relação à cor de fundo, como texto cinzento claro sobre branco. O JeffCrunch mede o contraste do texto com o preenchimento. O texto pequeno precisa de um rácio de pelo menos 4,5 para 1, e o texto grande (18 pontos, ou 14 pontos a negrito) precisa de 3 para 1. Indica a primeira célula de cada combinação de tipo de letra e preenchimento. Isto é um aviso.
- Uma folha que ainda tem o nome que lhe foi dado, como Folha1. Isto é uma sugestão.

Prima **Verificar Novamente** depois de fazer alterações.
