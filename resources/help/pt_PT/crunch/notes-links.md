# Notas, comentários e ligações
<!-- keywords: remover hiperligações, remover aspeto de ligação, mover nota, dimensionar nota, redimensionar nota, caixa de nota, repor tamanho da nota, teclado, idioma, idioma de verificação, definir idioma, não verificar ortografia, traduzir, tradução, libretranslate, painel de comentários, novo comentário, responder, resolver, reabrir, conversa, comentário anterior, comentário seguinte, mostrar comentários, converter notas em comentários, converter comentários em notas, ortografia, verificação ortográfica, palavra errada, F7, ignorar tudo, alterar tudo, adicionar ao dicionário, dicionário de sinónimos, sinónimos, lista de palavras, nota, notas, comentário, comentários, passar o ponteiro, marca vermelha, canto, nova nota, editar nota, eliminar nota, mostrar nota, ocultar nota, mostrar todas as notas, ligação, ligações, hiperligação, endereço, página web, correio eletrónico, lugar neste livro, Ctrl+K, Ctrl+clique, dica de ecrã, abrir ligação, remover ligação, conversa, estatísticas do livro, contar, células, fórmulas, gráficos, tabelas dinâmicas, acessibilidade, texto alternativo, células unidas, ficheiros recentes -->

## Notas

Uma **nota** é um conjunto de palavras guardado com uma célula. Uma célula que tenha uma nota mostra uma pequena marca vermelha no canto superior direito. Quando posiciona o ponteiro sobre a célula, a nota aparece ao lado dela; quando afasta o ponteiro, a nota desaparece.

Para colocar uma nota numa célula, selecione-a e use **Revisão > Nova Nota** (ou prima **Shift+F2**). Escreva as palavras e clique em **OK**. Numa célula que já tem nota, o mesmo comando aparece como **Editar Nota**. Se apagar as palavras, a nota é removida.

- **Eliminar Nota** remove as notas das células selecionadas.
- **Mostrar ou Ocultar Nota** mantém a nota das células selecionadas visível na folha o tempo todo, numa caixa ao lado de cada célula, ou volta a ocultá-la.
- **Mostrar Todas as Notas** faz o mesmo para todas as notas da folha; o mesmo botão passa então a chamar-se **Ocultar Todas as Notas**.
- **Nota Anterior** e **Nota Seguinte** deslocam-se de uma célula com nota para a seguinte.
- **Mover ou Dimensionar Nota** move ou dimensiona com as teclas a caixa de uma nota visível. Selecione a célula e use o comando (**Revisão > Notas**, ou clique com o botão direito na célula). Depois, as **teclas de seta** movem a caixa 10 píxeis, **Shift com uma seta** torna-a mais larga, mais estreita, mais alta ou mais baixa em 10 píxeis, e **Ctrl** (**Option** num Mac) faz os passos de 1 píxel. As combinações com Alt ou Meta (ou Command, num Mac) não fazem nada entretanto, e qualquer outro comando, uma mudança de folha, um Anular ou um Refazer faz a caixa ser abandonada. **Enter** mantém a caixa onde está, como um passo que pode anular, e **Esc** abandona a operação. Um leitor de ecrã indica a posição da caixa após cada tecla, em píxeis com zoom a 100% (a distância horizontal e vertical a partir do canto superior esquerdo da célula, e a largura e a altura da caixa), e indica-a quando solta uma tecla que tinha premido, e não a cada repetição; também indica quando a caixa é mantida ou abandonada. Uma caixa tem no máximo 640 píxeis de largura e 480 de altura, e começa a menos de 1000 píxeis da sua célula, tal como quando a arrasta. Arrastar a caixa e as suas oito alças com o ponteiro produz o mesmo efeito.
- **Repor Tamanho da Nota** repõe a caixa da nota no valor predefinido: ao lado da célula, com o tamanho necessário para as suas palavras.

Cada uma destas ações é um passo que pode anular. Uma nota acompanha a sua célula quando linhas ou colunas são inseridas ou eliminadas, e quando as células são ordenadas. **Limpar Tudo** remove as notas e os comentários das células que limpa.

Uma célula também pode ter uma conversa de **comentários**, com respostas. Veja a parte seguinte.

As notas podem ser guardadas num livro .xlsx, num livro .ods e num livro .xls. Um livro aberto a partir de um ficheiro .ods ou .xls mostra as respetivas notas, e pode acrescentar, alterar, mover e eliminar notas como num livro .xlsx. A data de uma nota .ods não é mantida quando o JeffCrunch guarda. Uma conversa de comentários guardada como ficheiro .ods ou .xls torna-se uma nota simples com as suas palavras, e continua a ser uma nota simples quando esse ficheiro é aberto. Um ficheiro .xls guarda a distância de uma caixa à célula com um tamanho fixo, por isso uma caixa situada muito à direita numa célula larga pode deslocar-se um pouco. Um livro aberto a partir de um ficheiro de texto não tem notas.

## Comentários

Um **comentário** é como uma nota, mas inicia uma **conversa**: as pessoas podem responder-lhe, e uma delas pode marcar a conversa como **resolvida** quando a pergunta estiver respondida. Cada comentário mostra quem o escreveu e quando. Uma célula com uma conversa tem a mesma marca vermelha de uma célula com nota. Posicione o ponteiro sobre a célula para ler a conversa completa, com cada resposta recuada por baixo do comentário a que responde. Uma conversa resolvida começa com a palavra **Resolvido**.

No separador **Revisão**, no grupo **Comentários**:

- **Novo Comentário** (**Ctrl+Shift+F2**) inicia uma conversa na célula selecionada. Escreva as palavras e clique em **OK**. Funciona numa célula sem nota e sem comentários.
- **Responder** acrescenta as suas palavras à conversa da célula selecionada.
- **Resolver Conversa** marca a conversa como concluída. O mesmo botão passa então a chamar-se **Reabrir Conversa**, que a volta a abrir.
- **Eliminar Comentário** remove as conversas das células selecionadas.
- **Comentário Anterior** e **Comentário Seguinte** deslocam-se de uma célula com conversa para a seguinte.
- **Mostrar Comentários** abre o painel **Comentários** ao lado da folha. Este lista todas as conversas da folha sob a forma de cartões: a célula, cada comentário com o respetivo autor e hora, e os botões **Responder**, **Resolver** ou **Reabrir**, e **Eliminar**. **Ir Para** move-o para a célula. O painel acompanha todas as alterações, incluindo o Anular.
- **Converter Notas em Comentários** transforma cada nota simples da folha numa conversa com um só comentário. **Converter Comentários em Notas** transforma cada conversa de novo numa nota. Uma nota não tem respostas, por isso os comentários de uma conversa passam a ser uma só nota com todas as suas palavras, e o JeffCrunch pede confirmação antes de o fazer numa conversa que tenha respostas.

Cada uma destas ações é um passo que pode anular.

Os programas que não conseguem mostrar conversas apresentam no seu lugar uma nota simples. O JeffCrunch mantém essa nota, com as palavras da conversa, sempre que guarda. As conversas, as respostas e as pessoas que as escreveram mantêm-se como estavam quando guarda e volta a abrir o livro. Uma conversa com mais de 500 comentários é apresentada e mantida, mas o JeffCrunch não lhe acrescenta nada nem a altera.

Os comentários são guardados num livro .xlsx. Um livro aberto a partir de um tipo de ficheiro mais antigo, ou de um ficheiro de texto, mantém os seus comentários apenas depois de o guardar como livro .xlsx.

## Ligações

Uma **ligação** leva-o a outro lugar quando a segue: a uma página web, a um ficheiro, a um endereço de correio eletrónico ou a um lugar neste livro (uma célula, células ou um nome).

Para colocar uma ligação numa célula, selecione-a e use **Inserir > Ligação** (ou prima **Ctrl+K**). Escolha para onde vai, escreva o endereço ou o lugar (ou escolha um dos ficheiros abertos recentemente em **Ou um ficheiro recente**) e, se quiser, as palavras que a célula deve mostrar e uma **dica de ecrã**. Numa célula que já tem ligação, o mesmo comando aparece como **Editar Ligação**. **Remover Ligação** retira as ligações das células selecionadas. **Remover Hiperligações** (no menu **Limpar** do separador Base, e no menu do botão direito de uma célula com ligação) retira as ligações e também o aspeto que uma ligação dá à célula, a cor azul e o sublinhado, para que deixe de parecer uma ligação. Tudo o resto da célula se mantém: o preenchimento, o formato numérico, os limites, o alinhamento e o tipo de letra próprio. Uma célula que tenha pintado de azul e sublinhado manualmente, sem o estilo Hiperligação, mantém esse aspeto. Ambas as ações são um passo que pode anular, e uma folha protegida que não permita ligações recusa-as. As ligações e o aspeto são guardados com o livro, em todos os formatos que suportam ligações.

Posicione o ponteiro sobre uma célula com ligação para ver para onde leva. Para a seguir, mantenha **Ctrl** premido e clique na célula, ou selecione a célula e use **Abrir Ligação** no menu do botão direito. Um clique simples apenas seleciona a célula.

O JeffCrunch nunca abre nada por iniciativa própria. Uma página web ou um endereço de correio eletrónico abre-se no seu programa habitual quando segue a ligação. Uma ligação para um ficheiro, ou para outro tipo de endereço, pede-lhe confirmação primeiro. Uma ligação que executasse um script nunca é aberta.

Uma ligação acompanha a sua célula quando linhas ou colunas são inseridas ou eliminadas, e quando as células são ordenadas, e é guardada com o livro.

## Estatísticas do Livro

**Estatísticas do Livro** (separador **Revisão**) conta o que o livro contém: quantas **Folhas** tem, quantas **Células com dados** (células que contêm um número, texto ou fórmula), quantas **Fórmulas**, quantos **Gráficos** e quantas **Tabelas dinâmicas**. As folhas que eliminou não são contadas. Os mesmos números estão na página **Informações** do separador Ficheiro.

## Ortografia

**Ortografia** (separador **Revisão**, ou **F7**) verifica as palavras das suas células de texto, uma palavra errada de cada vez. Se tiver selecionado mais do que uma célula, verifica apenas essas células. Caso contrário, verifica a folha inteira. Não analisa fórmulas, números nem datas.

A janela mostra a palavra que não consta do dicionário, uma caixa chamada **Alterar para** com a melhor sugestão e uma lista de outras sugestões. Escolha uma sugestão ou escreva a sua própria palavra e, depois, escolha:

- **Alterar** coloca a nova palavra no lugar da errada nessa célula. Cada alteração é um passo que pode anular.
- **Alterar Tudo** coloca-a no lugar da mesma palavra em todas as células verificadas, como um só passo que pode anular.
- **Ignorar** deixa a palavra como está desta vez. **Ignorar Tudo** deixa-a como está em todo o lado até fechar o JeffCrunch.
- **Adicionar** coloca a palavra no seu dicionário pessoal, para que nunca mais seja assinalada. O seu dicionário é partilhado com as outras aplicações do JeffOffice.

**Ficheiro > Opções > Verificação** decide se as palavras em maiúsculas e as palavras com números são verificadas. O JeffCrunch verifica no idioma em que é apresentado, se tiver dicionário para ele, e em inglês dos Estados Unidos se não tiver.

Enquanto escreve numa célula, uma palavra que não conste do dicionário fica com uma linha vermelha ondulada por baixo. Clique com o botão direito na palavra para escolher uma correção, ou para escolher **Ignorar Tudo** ou **Adicionar ao Dicionário**. As células da grelha não são sublinhadas, e uma entrada que comece com um sinal de igual nunca é verificada. Para ver, editar, importar ou exportar a sua lista de palavras acrescentadas, e para listar palavras que são sempre assinaladas como erradas, clique em **Ficheiro > Opções > Verificação > Dicionário Personalizado**. As suas listas são as mesmas em todas as aplicações do JeffOffice.

## Definir o idioma de algumas células

A ortografia verifica cada célula no seu próprio idioma. Para o alterar, selecione as células e clique em **Definir Idioma de Verificação** no grupo **Idioma** do separador **Revisão**. Escolha o idioma na lista e, depois, clique em **OK**. Uma célula que não tenha marcado é verificada no idioma definido em **Ficheiro > Opções**. Uma folha de cálculo OpenDocument (.ods) mantém o idioma de cada célula. Um livro .xlsx não tem onde o guardar, por isso o idioma só se mantém enquanto o livro estiver aberto.

## Não verificar algumas células

Há texto que não deve ser verificado: um código, um nome ou uma linha numa língua inventada. Selecione as células e clique em **Não Verificar Ortografia** no grupo **Idioma**. **Ortografia** e as linhas vermelhas onduladas da célula ignoram-nas. Clique novamente para voltar a verificar as células. Um ficheiro .ods mantém esta marca, e um livro .xlsx mantém-na apenas enquanto o livro estiver aberto.

## Traduzir texto

1. Selecione as células, ou selecione algumas das palavras que está a escrever numa célula. O texto das células selecionadas é enviado, uma linha de cada vez.
2. Clique em **Traduzir** no grupo **Idioma**.
3. Escolha o idioma para o qual traduzir.
4. Clique em **OK**.

O seu navegador web abre então um serviço de tradução chamado LibreTranslate, com o seu texto nele. Nada é enviado até clicar em **OK**. Pode alterar o endereço do serviço na caixa, por exemplo para um que execute no seu próprio computador. Só são utilizados endereços web seguros (https), e endereços http para este computador. São enviadas no máximo 5000 letras.

## Dicionário de sinónimos

**Dicionário de Sinónimos** (separador **Revisão**, ou **Shift+F7**) encontra palavras com um significado quase igual ao de uma palavra numa célula. Procura a primeira palavra do texto da célula em que está. Escreva outra palavra em **Procurar** e prima **Enter** para procurar essa em vez disso. Escolha uma palavra na lista e clique em **Substituir**, ou faça duplo clique nela. A palavra da célula é substituída, e uma primeira letra maiúscula mantém-se maiúscula. **Procurar** procura a palavra que escolheu, para seguir uma cadeia de significados. A lista de palavras funciona em inglês.

## Verificar Acessibilidade

**Verificar Acessibilidade** (separador **Revisão**) abre uma pequena janela que lista o que no livro é difícil de usar para alguns leitores, ou para um leitor de ecrã (um programa que lê o ecrã em voz alta). Cada linha começa por **Erro**, **Aviso** ou **Sugestão**. Clique numa linha para ir para a célula.

Procura:

- Um gráfico ou uma imagem sem **texto alternativo** (as palavras que descrevem o que mostra). Isto é um erro.
- Uma tabela sem linha de cabeçalho, por isso as suas colunas não têm nomes. Isto é um erro.
- **Células unidas**, que são difíceis de percorrer com o teclado. Isto é um aviso.
- Texto que é difícil de ler sobre a cor de fundo, como texto cinzento claro sobre branco. O JeffCrunch mede o contraste do texto em relação ao preenchimento. O texto pequeno precisa de uma razão de pelo menos 4,5 para 1, e o texto grande (18 pontos, ou 14 pontos a negrito) precisa de 3 para 1. Indica a primeira célula de cada combinação de tipo de letra e preenchimento. Isto é um aviso.
- Uma folha que ainda tem o nome que lhe foi dado, como Folha1. Isto é uma sugestão.

Prima **Verificar Novamente** depois de fazer alterações.
