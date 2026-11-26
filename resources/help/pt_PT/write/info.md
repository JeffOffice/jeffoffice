# Informações: propriedades, palavras-passe e Inspecionar

<!-- keywords: verificar acessibilidade, informações, propriedades, título, autor, assunto, palavras-chave, categorias, comentários, empresa, gestor, propriedades personalizadas, docproperty, marcar como final, final, só de leitura, palavra-passe, encriptar, proteger, palavra-passe de abertura, inspecionar documento, informações pessoais, texto oculto, XML personalizado, verificar compatibilidade, converter, modo de compatibilidade, histórico de versões, versões, guardar uma versão, voltar, restaurar, comparar versões, mostrar na pasta, copiar caminho, pasta, caminho -->

Clique em **Ficheiro > Informações** para ver os dados do seu documento e para o proteger antes de o partilhar. Prima **Esc** para voltar ao documento.

## Onde está o ficheiro

No topo, a página mostra onde o ficheiro está guardado. Clique em **Mostrar na Pasta** para abrir essa pasta. Clique em **Copiar Caminho** para copiar o caminho, de modo a poder colá-lo num correio eletrónico ou numa conversa.

## Propriedades

As propriedades são dados sobre o documento, como o título e quem o escreveu. A página tem uma caixa para cada uma: **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gestor**. Escreva numa caixa e prima **Tab** ou **Enter**. O JeffWrite mantém a alteração e guarda-a na próxima vez que gravar o documento.

Por baixo das caixas, a página mostra o que o JeffWrite calcula: quando o documento foi criado, guardado e impresso, e quantas páginas, palavras, carateres e parágrafos tem.

### As suas propriedades

Clique em **Propriedades Personalizadas** para criar propriedades próprias, como **Projeto** ou **Cliente**. Clique em **Adicionar**, escreva um nome, escolha o tipo de valor (texto, um número inteiro, um número, sim ou não, ou uma data) e escreva o valor.

Um campo pode mostrar uma propriedade personalizada. Clique em **Inserir > Campo** e escolha um campo. Altere o código para `DOCPROPERTY "Cliente"` e clique em **OK**. O texto mostra o valor. Prima **F9** para o atualizar depois de alterar a propriedade. Veja [Campos](fields).

## Marcar como Final

Clique em **Marcar como Final** para indicar aos leitores que o documento está concluído. O JeffWrite guarda o documento e depois mantém-no apenas para leitura. Quando alguém o abre, a barra de título indica que é só de leitura, e uma nota informa que foi marcado como final. É uma forma suave de dizer "por favor, não altere isto". Não é um bloqueio, porque qualquer pessoa pode escolher **Marcado como Final** novamente para o editar.

## Abrir Sempre Só para Leitura

Clique em **Abrir Sempre Só para Leitura** para que o documento seja aberto apenas para leitura em cada abertura, de modo a que ninguém o altere por engano. A escolha fica guardada com o ficheiro quando o guarda. Na próxima vez que alguém abrir o ficheiro, a barra de estado indica que está aberto apenas para leitura. Para o editar, abra **Ficheiro > Informações** e clique em **Abrir Sempre Só para Leitura**. Basta um clique, por isso é um lembrete e não um bloqueio. Outros programas não leem esta escolha, e um ficheiro .odt não a pode guardar; por isso, guarde primeiro o documento como ficheiro .docx. **Verificar Compatibilidade** indica esta situação para um ficheiro .odt.

## Encriptar com Palavra-passe

Clique em **Encriptar com Palavra-passe** para pedir uma palavra-passe sempre que o ficheiro for aberto. Escreva a palavra-passe duas vezes e clique em **Proteger**. O ficheiro fica codificado quando o guardar, para que ninguém o possa ler sem a palavra-passe. O JeffWrite usa o tipo de proteção forte que outros processadores de texto usam, por isso esses programas também conseguem abrir o ficheiro com a palavra-passe.

- Uma palavra-passe **não pode ser recuperada** se a esquecer. Guarde-a num local seguro.
- As versões que guardou e a cópia de segurança seguem a palavra-passe: ficam protegidas com ela e mudam quando a alterar ou remover.
- Para alterar a palavra-passe, clique novamente no mesmo cartão. Clique em **Remover Palavra-passe** para a retirar. A alteração fica guardada na próxima vez que guardar.
- As palavras-passe funcionam para ficheiros **.docx** e para ficheiros **.odt**. Um ficheiro .odt é protegido da forma descrita pela norma OpenDocument, por isso outros programas que sigam a norma também o conseguem abrir com a palavra-passe. Um ficheiro de texto simples ou .rtf não pode guardar uma palavra-passe, pelo que o JeffWrite não guarda um documento protegido nesses formatos. Em vez disso, avisa-o, em vez de guardar sem a palavra-passe.

Para abrir um ficheiro protegido, abra-o como de costume. O JeffWrite pede a palavra-passe. Se não estiver correta, pode tentar novamente ou clicar em **Cancelar**.

O JeffWrite também abre um ficheiro **.doc** antigo que peça uma palavra-passe. Pede-a da mesma forma. O JeffWrite nunca guarda ficheiros .doc, por isso **Guardar Como** pede um novo nome, e a cópia .docx mantém a mesma palavra-passe.

## Inspecionar Documento

Clique em **Inspecionar Documento** antes de enviar um documento a alguém. A ferramenta lista o que o ficheiro contém e que poderá não querer partilhar, e quantos elementos de cada tipo encontrou:

- **Comentários** são notas que as pessoas deixaram na margem.
- **Alterações registadas** são edições que ainda não foram aceites nem rejeitadas. **Aceitar Tudo** mantém cada alteração como parte do texto.
- **Propriedades do documento e informações pessoais** são o autor, a pessoa que o guardou pela última vez, a empresa e o gestor.
- **Texto oculto** é texto definido como oculto. Não é mostrado nem impresso, mas está no ficheiro.
- **Cabeçalhos e rodapés** são o texto no topo e na base das páginas.
- **Dados XML personalizados** são dados que outros programas acrescentaram ao ficheiro. Não são visíveis numa página.
- **Ligações externas** são ligações a outros ficheiros, não ligações Web: uma imagem ou um objeto que está apenas ligado (mostra um ficheiro que não está no documento), e um campo que obtém o texto ou a imagem de outro ficheiro (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE e DDEAUTO). **Remover Tudo** transforma cada campo no texto que mostra neste momento, retira cada imagem ou objeto ligado e deixa a ligação de fora do ficheiro quando o guardar. Os outros ficheiros não são alterados. As ligações em cabeçalhos, rodapés, notas de rodapé e notas de fim também são contadas e removidas. As ligações Web mantêm-se. As ligações dentro de comentários, caixas de texto e das partes que o programa mantém como estão (como o texto dos desenhos) também são procuradas. Uma imagem ligada que tenha também a sua própria cópia da imagem mantém-se, como imagem sem ligação, e a ligação fica de fora do ficheiro quando o guardar. Remover ligações dentro de comentários e de outras partes mantidas como estão não pode ser anulado; as caixas de texto são um passo de Anular. O modelo a que o documento está associado (o seu caminho contém muitas vezes um nome de utilizador) também conta como ligação externa: **Remover Tudo** desassocia-o, e o documento mantém os seus próprios estilos. As outras ligações a ficheiros não são retiradas, e uma linha por baixo da contagem indica-as: a origem de dados da impressão em série, as molduras, os subdocumentos dentro de outras partes, os dados externos de um gráfico, os filmes e sons ligados e as folhas de estilo ligadas. Esta linha conta também qualquer outra ligação num campo, num comentário, numa caixa de texto ou na lista de ligações do ficheiro que nada no documento utiliza, e indica quando o endereço de uma imagem ligada não está no ficheiro ou quando uma parte está numa codificação que a verificação não consegue ler. Nesse caso, a linha diz "Não totalmente verificado" em vez de "Nenhum encontrado". Uma única ação de **Remover Tudo** retira no máximo 100 000 ligações nas partes mantidas; se restarem mais, a linha indica quantas, e pode executar **Remover Tudo** novamente.
- **Imagens originais mantidas sob efeitos de imagem** são as imagens a partir das quais foi criado um efeito, como Desfocagem ou Mosaico. O ficheiro guarda o original ao lado da imagem final, e quem receber o ficheiro pode extraí-lo. **Remover Originais** mantém apenas a imagem final, e **Repor Imagem** deixa então de conseguir repor o original.
- **Partes recortadas de imagens** são as partes que recortou e que deixaram de ser visíveis. Continuam na imagem, e quem receber o ficheiro pode vê-las. **Remover Partes Recortadas** elimina-as, e a página fica exatamente como estava. Uma imagem com efeito passa primeiro a ser a sua imagem final.

Clique em **Remover Tudo** ao lado de um tipo para o retirar. Pode anular as alterações ao texto com **Ctrl+Z**. As ligações retiradas de comentários e de outras partes mantidas, bem como os dados XML personalizados, não podem ser anulados. Guarde o documento depois, para manter a cópia limpa.

**Remover informações pessoais deste ficheiro ao guardar**, por baixo dos cartões, faz isso sempre que guardar, em qualquer ficheiro, sem alterar o documento que tem aberto. É a mesma caixa que está em **Opções**, e é igual em todas as aplicações JeffOffice.

## Converter

Um ficheiro .docx que indica ter sido criado segundo a forma de um programa mais antigo mostra **[Modo de Compatibilidade]** depois do nome na barra de título. Isso significa que o documento é disposto segundo as regras do programa mais antigo. Um documento novo, um ficheiro que não o indique, e um ficheiro .odt, .rtf, .doc ou de texto não mostram nada. Abra **Ficheiro > Informações** e clique em **Converter** para usar as regras mais recentes. O programa pede confirmação, e a ação não pode ser anulada com Ctrl+Z. As páginas passam logo a ter o aspeto que terão quando guardar o ficheiro e o voltar a abrir. A conversão altera estas coisas:

- **Tabelas.** No modo antigo, o avanço de uma tabela é medido até ao texto da primeira célula, pelo que a margem esquerda da tabela fica ligeiramente à esquerda desse texto. No modo mais recente, o avanço é medido até à margem da tabela, pelo que as tabelas com avanço se deslocam para a direita na largura da margem da célula.
- **Quebras de página.** No modo antigo, um parágrafo que termina com uma quebra de página deixa o parágrafo seguinte começar a nova página. No modo mais recente, o fim desse parágrafo passa para a nova página e deixa uma linha vazia no seu topo.
- **Alturas de linha.** Um ficheiro criado no modo mais antigo pode pedir linhas com um número inteiro de pontos. Depois da conversão, as linhas têm a sua altura natural, pelo que as linhas e as páginas podem quebrar noutros sítios.

Da próxima vez que guardar o ficheiro, ele indica que está no modo mais recente, e a barra de título deixa de mostrar a etiqueta. As escolhas que o ficheiro define uma a uma, como a hifenização e as páginas espelhadas, mantêm-se como estão. Um documento que já está no modo mais recente não tem o botão **Converter**.

## Verificar Compatibilidade

Clique em **Verificar Compatibilidade** e escolha um tipo de ficheiro para ver o que ele não consegue guardar. O JeffWrite guarda ficheiros **.docx**, **.odt**, **.rtf** e **.txt**. Um ficheiro .docx guarda tudo o que existe no seu documento. Os outros tipos não conseguem guardar algumas coisas, e esta lista indica quais. Indica também as escolhas guardadas no próprio ficheiro, Marcar como Final e Abrir Sempre Só para Leitura, quando estão ativas, e a palavra-passe para um tipo que não a consegue guardar. Se nada estiver listado, não se perde nada.

Ao guardar como ficheiro .odt, o JeffWrite lista o que o ficheiro não consegue guardar **antes** de escrever seja o que for. Pode escolher **Guardar Mesmo Assim**, **Guardar como .docx** (o ficheiro .docx é colocado ao lado dele, e nada se perde) ou **Cancelar**. Marque **Não voltar a perguntar para este ficheiro** e o JeffWrite deixa de perguntar sobre esse ficheiro até fechar o programa.

## Verificar Acessibilidade

Clique em **Verificar Acessibilidade** para abrir a lista de elementos que podem ser difíceis de ler ou usar para algumas pessoas, como uma imagem sem descrição. É a mesma verificação que **Rever > Verificar Acessibilidade**. Veja [Contar palavras, definir o idioma, verificar acessibilidade e procurar palavras](proofing-tools).

## Histórico de Versões

Clique em **Histórico de Versões** para guardar cópias com nome do documento no seu computador. Nada é enviado para lado nenhum. Uma versão é uma cópia completa do documento tal como está naquele momento. Guarde uma antes de uma alteração grande, para poder voltar a ela. O documento tem de estar guardado primeiro, porque as versões são guardadas com o nome do ficheiro.

- **Guardar uma Versão** pede um nome, como "Antes da revisão", e guarda uma cópia.
- **Abrir uma Cópia** abre a versão escolhida numa nova janela. A própria versão não é alterada.
- **Comparar** coloca o documento e a versão escolhida lado a lado, como um só documento, com as diferenças mostradas como alterações registadas. Veja [Controlar alterações](track-changes).
- **Voltar a Esta Versão** torna o documento igual à versão escolhida. O JeffWrite guarda primeiro o documento tal como estava, como nova versão, para que nada se perca. Guarde o documento para manter a alteração.
- **Eliminar** remove a versão escolhida.

O JeffWrite guarda até 50 versões de um ficheiro e elimina a mais antiga quando existem mais.
