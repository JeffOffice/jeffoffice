# Notas, comentários e ligações

<!-- keywords: painel de comentários, novo comentário, responder, resolver, reabrir, conversa, comentário anterior, comentário seguinte, mostrar comentários, converter notas em comentários, converter comentários em notas, dicionário de sinónimos, sinónimos, lista de palavras, nota, notas, comentário, comentários, passar o ponteiro, marca vermelha, canto, nova nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas as notas, ligação, ligações, hiperligação, endereço, página web, correio eletrónico, local neste livro, Ctrl+K, Ctrl+clique, descrição, abrir ligação, remover ligação, conversa, estatísticas do livro, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células unidas, ficheiros recentes -->

## Notas

Uma **nota** é um pequeno texto associado a uma célula. Uma célula que tenha uma nota mostra uma pequena marca vermelha no canto superior direito. Quando coloca o ponteiro sobre a célula, a nota aparece ao lado dela; quando afasta o ponteiro, a nota desaparece.

Para colocar uma nota numa célula, selecione-a e utilize **Revisão > Nova Nota** (ou prima **Shift+F2**). Escreva o texto e clique em **OK**. Numa célula que já tem uma nota, o mesmo comando aparece como **Editar Nota**. Se apagar o texto, a nota é removida.

- **Eliminar Nota** remove as notas das células selecionadas.
- **Mostrar ou Ocultar Nota** mantém a nota das células selecionadas sempre visível na folha, numa caixa ao lado de cada célula, ou volta a ocultá-la.
- **Mostrar Todas as Notas** faz o mesmo para todas as notas da folha; o mesmo botão passa então a chamar-se **Ocultar Todas as Notas**.
- **Nota Anterior** e **Nota Seguinte** passam de uma célula com nota para a seguinte.

Cada uma destas ações pode ser anulada com um único passo. Uma nota acompanha a respetiva célula quando são inseridas ou eliminadas linhas ou colunas, e quando as células são ordenadas. **Limpar Tudo** remove as notas e os comentários das células que limpa.

Uma célula também pode ter uma conversa de **comentários**, com respostas. Consulte a secção seguinte.

As notas podem ser guardadas num livro .xlsx. Um livro aberto a partir de um tipo de ficheiro mais antigo, ou de um ficheiro de texto, só mantém as notas depois de o guardar como livro .xlsx.

## Comentários

Um **comentário** é semelhante a uma nota, mas inicia uma **conversa**: as pessoas podem responder-lhe e uma delas pode marcar a conversa como **resolvida** quando a dúvida estiver esclarecida. Cada comentário mostra quem o escreveu e quando. Uma célula com uma conversa tem a mesma marca vermelha que uma célula com nota. Coloque o ponteiro sobre a célula para ler a conversa completa, com cada resposta recuada por baixo do comentário a que responde. Uma conversa resolvida começa com a palavra **Resolvido**.

No separador **Revisão**, no grupo **Comentários**:

- **Novo Comentário** (**Ctrl+Shift+F2**) inicia uma conversa na célula selecionada. Escreva o texto e clique em **OK**. Funciona numa célula sem notas nem comentários.
- **Responder** acrescenta o seu texto à conversa da célula selecionada.
- **Resolver Conversa** marca a conversa como concluída. O mesmo botão passa então a chamar-se **Reabrir Conversa**, que a volta a abrir.
- **Eliminar Comentário** remove as conversas das células selecionadas.
- **Comentário Anterior** e **Comentário Seguinte** passam de uma célula com conversa para a seguinte.
- **Mostrar Comentários** abre o painel **Comentários** ao lado da folha. Este painel lista todas as conversas da folha sob a forma de cartões: a célula, cada comentário com o respetivo autor e hora, e os botões **Responder**, **Resolver** ou **Reabrir**, e **Eliminar**. **Ir Para** move para a célula. O painel acompanha todas as alterações, incluindo a ação de **Anular**.
- **Converter Notas em Comentários** transforma cada nota simples da folha numa conversa com um único comentário. **Converter Comentários em Notas** transforma cada conversa de novo numa nota. Uma nota não tem respostas, por isso os comentários de uma conversa passam a ser uma única nota com todo o seu texto, e o JeffCrunch pede confirmação antes de o fazer numa conversa que tenha respostas.

Cada uma destas ações pode ser anulada com um único passo.

Os programas que não conseguem mostrar conversas apresentam, em vez delas, uma nota simples. O JeffCrunch mantém essa nota, com o texto da conversa, sempre que guarda o livro. As conversas, as respostas e as pessoas que as escreveram mantêm-se tal como estavam quando guarda e volta a abrir o livro. Uma conversa com mais de 500 comentários é apresentada e mantida, mas o JeffCrunch não lhe acrescenta comentários nem a altera.

Os comentários são guardados num livro .xlsx. Um livro aberto a partir de um tipo de ficheiro mais antigo, ou de um ficheiro de texto, só mantém os comentários depois de o guardar como livro .xlsx.

## Ligações

Uma **ligação** leva-o a outro local quando a segue: a uma página web, a um ficheiro, a um endereço de correio eletrónico ou a um local neste livro (uma célula, células ou um nome).

Para colocar uma ligação numa célula, selecione-a e utilize **Inserir > Ligação** (ou prima **Ctrl+K**). Escolha o destino, escreva o endereço ou o local (ou escolha um dos ficheiros abertos recentemente em **Ou um ficheiro recente**) e, se quiser, o texto que a célula deve mostrar e uma **descrição**. Numa célula que já tem uma ligação, o mesmo comando aparece como **Editar Ligação**. **Remover Ligação** retira as ligações das células selecionadas.

Coloque o ponteiro sobre uma célula com ligação para ver para onde leva. Para a seguir, mantenha **Ctrl** premida e clique na célula, ou selecione a célula e utilize **Abrir Ligação** no menu de contexto (botão direito do rato). Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por iniciativa própria. Uma página web ou um endereço de correio eletrónico abre-se no seu programa habitual quando segue a ligação. Uma ligação para um ficheiro, ou para outro tipo de endereço, pede-lhe confirmação primeiro. Uma ligação que executaria um script nunca é aberta.

Uma ligação acompanha a respetiva célula quando são inseridas ou eliminadas linhas ou colunas, e quando as células são ordenadas, e é guardada com o livro.

## Estatísticas do Livro

**Estatísticas do Livro** (separador **Revisão**) conta o que o livro contém: quantas **Folhas** tem, quantas **Células com dados** (células que contêm um número, um texto ou uma fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. As folhas eliminadas não são contadas. Os mesmos números estão na página **Informações** do separador Ficheiro.

## Dicionário de Sinónimos

**Dicionário de Sinónimos** (separador **Revisão**, ou **Shift+F7**) encontra palavras com um significado quase igual ao de uma palavra existente numa célula. Procura a primeira palavra do texto da célula em que se encontra. Escreva outra palavra em **Procurar** e prima **Enter** para procurar essa em vez da anterior. Escolha uma palavra na lista e clique em **Substituir**, ou faça duplo clique nela. A palavra da célula é substituída, e uma inicial maiúscula mantém-se maiúscula. **Procurar** pesquisa a palavra que escolheu, para seguir uma cadeia de significados. A lista de palavras funciona em inglês.

## Verificar Acessibilidade

**Verificar Acessibilidade** (separador **Revisão**) abre uma pequena janela com o que, no livro, é difícil de usar para alguns leitores, ou para um leitor de ecrã (um programa que lê o ecrã em voz alta). Cada linha começa por **Erro**, **Aviso** ou **Sugestão**. Clique numa linha para ir para a célula.

Procura:

- Um gráfico ou uma imagem sem **texto alternativo** (as palavras que descrevem o que mostra). Isto é um erro.
- Uma tabela sem linha de cabeçalho, pelo que as respetivas colunas não têm nomes. Isto é um erro.
- **Células unidas**, que são difíceis de percorrer com o teclado. Isto é um aviso.
- Texto difícil de ler sobre a cor de fundo, como texto cinzento claro sobre branco. O JeffCrunch mede o contraste entre o texto e o preenchimento. O texto pequeno precisa de um rácio de pelo menos 4,5 para 1, e o texto grande (18 pontos, ou 14 pontos a negrito) precisa de 3 para 1. Indica a primeira célula de cada combinação de tipo de letra e preenchimento. Isto é um aviso.
- Uma folha que ainda tem o nome que lhe foi atribuído, como Folha1. Isto é uma sugestão.

Prima **Verificar Novamente** depois de fazer alterações.
